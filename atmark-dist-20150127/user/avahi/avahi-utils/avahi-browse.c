/* $Id: avahi-browse.c 1094 2006-01-22 17:25:07Z lennart $ */

/***
  This file is part of avahi.
 
  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.
 
  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <locale.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>
#include <avahi-common/llist.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include "sigint.h"

#ifdef HAVE_GDBM
#include "stdb.h"
#endif

typedef enum {
    COMMAND_HELP,
    COMMAND_VERSION,
    COMMAND_BROWSE_SERVICES,
    COMMAND_BROWSE_ALL_SERVICES,
    COMMAND_BROWSE_DOMAINS
} Command;

typedef struct Config {
    int verbose;
    int terminate_on_all_for_now;
    int terminate_on_cache_exhausted;
    char *domain;
    char *stype;
    int ignore_local;
    Command command;
    int resolve;
    int no_fail;
#ifdef HAVE_GDBM
    int no_db_lookup;
#endif
} Config;

typedef struct ServiceInfo ServiceInfo;

struct ServiceInfo {
    AvahiIfIndex interface;
    AvahiProtocol protocol;
    char *name, *type, *domain;

    AvahiServiceResolver *resolver;
    Config *config;

    AVAHI_LLIST_FIELDS(ServiceInfo, info);
};

static AvahiSimplePoll *simple_poll = NULL;
static AvahiClient *client = NULL;
static int n_all_for_now = 0, n_cache_exhausted = 0, n_resolving = 0;
static AvahiStringList *browsed_types = NULL;
static ServiceInfo *services = NULL;
static int n_columns = 80;
static int browsing = 0;

static void check_terminate(Config *c) {

    assert(n_all_for_now >= 0);
    assert(n_cache_exhausted >= 0);
    assert(n_resolving >= 0);
    
    if (n_all_for_now <= 0 && n_resolving <= 0) {

        if (c->verbose) {
            printf(": All for now\n");
            n_all_for_now++; /* Make sure that this event is not repeated */
        }
        
        if (c->terminate_on_all_for_now)
            avahi_simple_poll_quit(simple_poll);
    }
    
    if (n_cache_exhausted <= 0 && n_resolving <= 0) {

        if (c->verbose) {
            printf(": Cache exhausted\n");
            n_cache_exhausted++; /* Make sure that this event is not repeated */
        }
        
        if (c->terminate_on_cache_exhausted)
            avahi_simple_poll_quit(simple_poll);
    }
}

static ServiceInfo *find_service(AvahiIfIndex interface, AvahiProtocol protocol, const char *name, const char *type, const char *domain) {
    ServiceInfo *i;

    for (i = services; i; i = i->info_next)
        if (i->interface == interface &&
            i->protocol == protocol &&
            strcasecmp(i->name, name) == 0 &&
            avahi_domain_equal(i->type, type) &&
            avahi_domain_equal(i->domain, domain))

            return i;

    return NULL;
}

static void print_service_line(Config *config, char c, AvahiIfIndex interface, AvahiProtocol protocol, const char *name, const char *type, const char *domain) {
    char ifname[IF_NAMESIZE];

#ifdef HAVE_GDBM
    if (!config->no_db_lookup)
        type = stdb_lookup(type);
#endif
    
    printf("%c %4s %4s %-*s %-20s %s\n",
           c,
           interface != AVAHI_IF_UNSPEC ? if_indextoname(interface, ifname) : "n/a",
           protocol != AVAHI_PROTO_UNSPEC ? avahi_proto_to_string(protocol) : "n/a", 
           n_columns-35, name, type, domain);
}

static void service_resolver_callback(
    AvahiServiceResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *a,
    uint16_t port,
    AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {
    
    ServiceInfo *i = userdata;
    
    assert(r);
    assert(i);

    switch (event) {
        case AVAHI_RESOLVER_FOUND: {
            char address[AVAHI_ADDRESS_STR_MAX], *t;

            avahi_address_snprint(address, sizeof(address), a);

            t = avahi_string_list_to_string(txt);

            print_service_line(i->config, '=', interface, protocol, name, type, domain);
            
            printf("   hostname = [%s]\n"
                   "   address = [%s]\n"
                   "   port = [%i]\n"
                   "   txt = [%s]\n",
                   host_name,
                   address,
                   port,
                   t);
            avahi_free(t);

            break;
        }

        case AVAHI_RESOLVER_FAILURE:
            
            fprintf(stderr, "Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(client)));
            break;
    }

    
    avahi_service_resolver_free(i->resolver);
    i->resolver = NULL;

    assert(n_resolving > 0);
    n_resolving--;
    check_terminate(i->config);
}

static ServiceInfo *add_service(Config *c, AvahiIfIndex interface, AvahiProtocol protocol, const char *name, const char *type, const char *domain) {
    ServiceInfo *i;

    i = avahi_new(ServiceInfo, 1);

    if (c->resolve) {
        if (!(i->resolver = avahi_service_resolver_new(client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, service_resolver_callback, i))) {
            avahi_free(i);
            fprintf(stderr, "Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(client)));
            return NULL;
        }

        n_resolving++;
    } else
        i->resolver = NULL;

    i->interface = interface;
    i->protocol = protocol;
    i->name = avahi_strdup(name);
    i->type = avahi_strdup(type);
    i->domain = avahi_strdup(domain);
    i->config = c;

    AVAHI_LLIST_PREPEND(ServiceInfo, info, services, i);

    return i;
}

static void remove_service(Config *c, ServiceInfo *i) {
    assert(c);
    assert(i);

    AVAHI_LLIST_REMOVE(ServiceInfo, info, services, i);

    if (i->resolver)
        avahi_service_resolver_free(i->resolver);
    
    avahi_free(i->name);
    avahi_free(i->type);
    avahi_free(i->domain);
    avahi_free(i);
}

static void service_browser_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void *userdata) {

    Config *c = userdata;
    
    assert(b);
    assert(c);

    switch (event) {
        case AVAHI_BROWSER_NEW: {
            if (c->ignore_local && (flags & AVAHI_LOOKUP_RESULT_LOCAL))
                break;

            if (find_service(interface, protocol, name, type, domain))
                return;

            add_service(c, interface, protocol, name, type, domain);

            print_service_line(c, '+', interface, protocol, name, type, domain);
            break;

        }

        case AVAHI_BROWSER_REMOVE: {
            ServiceInfo *info;
            
            if (!(info = find_service(interface, protocol, name, type, domain)))
                return;

            remove_service(c, info);
            
            print_service_line(c, '-', interface, protocol, name, type, domain);
            break;
        }
            
        case AVAHI_BROWSER_FAILURE:
            fprintf(stderr, "service_browser failed: %s\n", avahi_strerror(avahi_client_errno(client)));
            avahi_simple_poll_quit(simple_poll);
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            n_cache_exhausted --;
            check_terminate(c);
            break;
            
        case AVAHI_BROWSER_ALL_FOR_NOW:
            n_all_for_now --;
            check_terminate(c);
            break;
    }
}

static void browse_service_type(Config *c, const char *stype, const char *domain) {
    AvahiServiceBrowser *b;
    AvahiStringList *i;
    
    assert(c);
    assert(client);
    assert(stype);

    for (i = browsed_types; i; i = i->next)
        if (avahi_domain_equal(stype, (char*) i->text))
            return;

    if (!(b = avahi_service_browser_new(
              client,
              AVAHI_IF_UNSPEC,
              AVAHI_PROTO_UNSPEC,
              stype,
              domain,
              0,
              service_browser_callback,
              c))) {

        fprintf(stderr, "avahi_service_browser_new() failed: %s\n", avahi_strerror(avahi_client_errno(client)));
        avahi_simple_poll_quit(simple_poll);
    }

    browsed_types = avahi_string_list_add(browsed_types, stype);

    n_all_for_now++;
    n_cache_exhausted++;
}

static void service_type_browser_callback(
    AvahiServiceTypeBrowser *b,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    Config *c = userdata;

    assert(b);
    assert(c);
    
    switch (event) {
        
        case AVAHI_BROWSER_NEW:
            browse_service_type(c, type, domain);
            break;

        case AVAHI_BROWSER_REMOVE:
            /* We're dirty and never remove the browser again */
            break;

        case AVAHI_BROWSER_FAILURE:
            fprintf(stderr, "service_type_browser failed: %s\n", avahi_strerror(avahi_client_errno(client)));
            avahi_simple_poll_quit(simple_poll);
            break;
            
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            n_cache_exhausted --;
            check_terminate(c);
            break;
            
        case AVAHI_BROWSER_ALL_FOR_NOW:
            n_all_for_now --;
            check_terminate(c);
            break;
    }
}

static void browse_all(Config *c) {
    AvahiServiceTypeBrowser *b;
    
    assert(c);

    if (!(b = avahi_service_type_browser_new(
              client,
              AVAHI_IF_UNSPEC,
              AVAHI_PROTO_UNSPEC,
              c->domain,
              0,
              service_type_browser_callback,
              c))) {
        
        fprintf(stderr, "avahi_service_type_browser_new() failed: %s\n", avahi_strerror(avahi_client_errno(client)));
        avahi_simple_poll_quit(simple_poll);
    }

    n_cache_exhausted++;
    n_all_for_now++;
}

static void domain_browser_callback(
    AvahiDomainBrowser *b,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    Config *c = userdata;

    assert(b);
    assert(c);
    
    switch (event) {
        
        case AVAHI_BROWSER_NEW:
        case AVAHI_BROWSER_REMOVE: {
            char ifname[IF_NAMESIZE];
            
            printf("%c %4s %4s %s\n",
                   event == AVAHI_BROWSER_NEW ? '+' : '-',
                   interface != AVAHI_IF_UNSPEC ? if_indextoname(interface, ifname) : "n/a",
                   protocol != AVAHI_PROTO_UNSPEC ? avahi_proto_to_string(protocol) : "n/a", 
                   domain);
            break;
        }

        case AVAHI_BROWSER_FAILURE:
            fprintf(stderr, "domain_browser failed: %s\n", avahi_strerror(avahi_client_errno(client)));
            avahi_simple_poll_quit(simple_poll);
            break;
            
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            n_cache_exhausted --;
            check_terminate(c);
            break;
            
        case AVAHI_BROWSER_ALL_FOR_NOW:
            n_all_for_now --;
            check_terminate(c);
            break;
    }
}

static void browse_domains(Config *c) {
    AvahiDomainBrowser *b;

    assert(c);

    if (!(b = avahi_domain_browser_new(
              client,
              AVAHI_IF_UNSPEC,
              AVAHI_PROTO_UNSPEC,
              c->domain,
              AVAHI_DOMAIN_BROWSER_BROWSE,
              0,
              domain_browser_callback,
              c))) {

        fprintf(stderr, "avahi_domain_browser_new() failed: %s\n", avahi_strerror(avahi_client_errno(client)));
        avahi_simple_poll_quit(simple_poll);
    }

    n_cache_exhausted++;
    n_all_for_now++;
}

static int start(Config *config) {

    assert(!browsing);
    
    if (config->verbose) {
        const char *version, *hn;
        
        if (!(version = avahi_client_get_version_string(client))) {
            fprintf(stderr, "Failed to query version string: %s\n", avahi_strerror(avahi_client_errno(client)));
            return -1;
        }
        
        if (!(hn = avahi_client_get_host_name_fqdn(client))) {
            fprintf(stderr, "Failed to query host name: %s\n", avahi_strerror(avahi_client_errno(client)));
            return -1;
        }
        
        fprintf(stderr, "Server version: %s; Host name: %s\n", version, hn);
        
        if (config->command == COMMAND_BROWSE_DOMAINS)
            fprintf(stderr, "E Ifce Prot Domain\n");
        else
            fprintf(stderr, "E Ifce Prot %-*s %-20s Domain\n", n_columns-35, "Name", "Type");
    }
    
    if (config->command == COMMAND_BROWSE_SERVICES)
        browse_service_type(config, config->stype, config->domain);
    else if (config->command == COMMAND_BROWSE_ALL_SERVICES)
        browse_all(config);
    else {
        assert(config->command == COMMAND_BROWSE_DOMAINS);
        browse_domains(config);
    }

    browsing = 1;
    return 0;
}
    
static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
    Config *config = userdata;

    /* This function might be called when avahi_client_new() has not
     * returned yet.*/
    client = c;
    
    switch (state) {
        case AVAHI_CLIENT_FAILURE:
            
            if (config->no_fail && avahi_client_errno(c) == AVAHI_ERR_DISCONNECTED) {
                int error;

                /* We have been disconnected, so let reconnect */

                fprintf(stderr, "Disconnected, reconnecting ...\n");

                avahi_client_free(client);
                client = NULL;

                avahi_string_list_free(browsed_types);
                browsed_types = NULL;
                
                while (services)
                    remove_service(config, services);

                browsing = 0;

                if (!(client = avahi_client_new(avahi_simple_poll_get(simple_poll), AVAHI_CLIENT_NO_FAIL, client_callback, config, &error))) {
                    fprintf(stderr, "Failed to create client object: %s\n", avahi_strerror(error));
                    avahi_simple_poll_quit(simple_poll);
                }

            } else {
                fprintf(stderr, "Client failure, exiting: %s\n", avahi_strerror(avahi_client_errno(c)));
                avahi_simple_poll_quit(simple_poll);
            }
            
            break;
            
        case AVAHI_CLIENT_S_REGISTERING:
        case AVAHI_CLIENT_S_RUNNING:
        case AVAHI_CLIENT_S_COLLISION:

            if (!browsing)
                if (start(config) < 0)
                    avahi_simple_poll_quit(simple_poll);

            break;
            
        case AVAHI_CLIENT_CONNECTING:
            
            if (config->verbose)
                fprintf(stderr, "Waiting for daemon ...\n");

            break;
    }
}

static void help(FILE *f, const char *argv0) {
    if (strstr(argv0, "domain"))
        fprintf(f, "%s [options] \n\n", argv0);
    else
        fprintf(f,
                "%s [options] <service type>\n"
                "%s [options] -a\n"
                "%s [options] -D\n\n",
                argv0, argv0, argv0);

            fprintf(f, 
            "    -h --help            Show this help\n"
            "    -V --version         Show version\n"
            "    -D --browse-domains  Browse for browsing domains instead of services\n"
            "    -a --all             Show all services, regardless of the type\n"
            "    -d --domain=DOMAIN   The domain to browse in\n"
            "    -v --verbose         Enable verbose mode\n"
            "    -t --terminate       Terminate after dumping a more or less complete list\n"
            "    -c --cache           Terminate after dumping all entries from the cache\n"
            "    -l --ignore-local    Ignore local services\n"
            "    -r --resolve         Resolve services found\n"
            "    -f --no-fail         Don't fail if the daemon is not available\n"
#ifdef HAVE_GDBM
            "    -k --no-db-lookup    Don't lookup service types\n"
#endif
            );
}

static int parse_command_line(Config *c, const char *argv0, int argc, char *argv[]) {
    int o;

    static const struct option long_options[] = {
        { "help",           no_argument,       NULL, 'h' },
        { "version",        no_argument,       NULL, 'V' },
        { "browse-domains", no_argument,       NULL, 'D' },
        { "domain",         required_argument, NULL, 'd' },
        { "all",            no_argument,       NULL, 'a' },
        { "verbose",        no_argument,       NULL, 'v' },
        { "terminate",      no_argument,       NULL, 't' },
        { "cache",          no_argument,       NULL, 'c' },
        { "ignore-local",   no_argument,       NULL, 'l' },
        { "resolve",        no_argument,       NULL, 'r' },
        { "no-fail",        no_argument,       NULL, 'f' },
#ifdef HAVE_GDBM
        { "no-db-lookup",   no_argument,       NULL, 'k' },
#endif
        { NULL, 0, NULL, 0 }
    };

    assert(c);

    c->command = strstr(argv0, "domain") ? COMMAND_BROWSE_DOMAINS : COMMAND_BROWSE_SERVICES;
    c->verbose =
        c->terminate_on_cache_exhausted =
        c->terminate_on_all_for_now =
        c->ignore_local =
        c->resolve =
        c->no_fail = 0;
    c->domain = c->stype = NULL;

#ifdef HAVE_GDBM
    c->no_db_lookup = 0;
#endif
    
    opterr = 0;
    while ((o = getopt_long(argc, argv, "hVd:avtclrDf"
#ifdef HAVE_GDBM
                            "k"
#endif
                            , long_options, NULL)) >= 0) {

        switch(o) {
            case 'h':
                c->command = COMMAND_HELP;
                break;
            case 'V':
                c->command = COMMAND_VERSION;
                break;
            case 'a':
                c->command = COMMAND_BROWSE_ALL_SERVICES;
                break;
            case 'D':
                c->command = COMMAND_BROWSE_DOMAINS;
                break;
            case 'd':
                avahi_free(c->domain);
                c->domain = avahi_strdup(optarg);
                break;
            case 'v':
                c->verbose = 1;
                break;
            case 't':
                c->terminate_on_all_for_now = 1;
                break;
            case 'c':
                c->terminate_on_cache_exhausted = 1;
                break;
            case 'l':
                c->ignore_local = 1;
                break;
            case 'r':
                c->resolve = 1;
                break;
            case 'f':
                c->no_fail = 1;
                break;
#ifdef HAVE_GDBM
            case 'k':
                c->no_db_lookup = 1;
                break;
#endif
            default:
                fprintf(stderr, "Invalid command line argument: %c\n", o);
                return -1;
        }
    }

    if (c->command == COMMAND_BROWSE_SERVICES) {
        if (optind >= argc) {
            fprintf(stderr, "Too few arguments\n");
            return -1;
        }

        c->stype = avahi_strdup(argv[optind]);
        optind++;
    }
    
    if (optind < argc) {
        fprintf(stderr, "Too many arguments\n");
        return -1;
    }
        
    return 0;
}

int main(int argc, char *argv[]) {
    int ret = 1, error;
    Config config;
    const char *argv0;
    char *ec;

    setlocale(LC_ALL, "");

    if ((argv0 = strrchr(argv[0], '/')))
        argv0++;
    else
        argv0 = argv[0];

    if ((ec = getenv("COLUMNS")))
        n_columns = atoi(ec);

    if (n_columns < 40)
        n_columns = 40;
    
    if (parse_command_line(&config, argv0, argc, argv) < 0)
        goto fail;

    switch (config.command) {
        case COMMAND_HELP:
            help(stdout, argv0);
            ret = 0;
            break;
            
        case COMMAND_VERSION:
            printf("%s "PACKAGE_VERSION"\n", argv0);
            ret = 0;
            break;

        case COMMAND_BROWSE_SERVICES:
        case COMMAND_BROWSE_ALL_SERVICES:
        case COMMAND_BROWSE_DOMAINS:
            
            if (!(simple_poll = avahi_simple_poll_new())) {
                fprintf(stderr, "Failed to create simple poll object.\n");
                goto fail;
            }
            
            if (sigint_install(simple_poll) < 0)
                goto fail;
            
            if (!(client = avahi_client_new(avahi_simple_poll_get(simple_poll), config.no_fail ? AVAHI_CLIENT_NO_FAIL : 0, client_callback, &config, &error))) {
                fprintf(stderr, "Failed to create client object: %s\n", avahi_strerror(error));
                goto fail;
            }
            
            avahi_simple_poll_loop(simple_poll);
            ret = 0;
            break;
    }
    
    
fail:

    while (services)
        remove_service(&config, services);

    if (client)
        avahi_client_free(client);

    sigint_uninstall();
    
    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    avahi_free(config.domain);
    avahi_free(config.stype);

    avahi_string_list_free(browsed_types);

#ifdef HAVE_GDBM
    stdb_shutdown();
#endif    

    return ret;
}
