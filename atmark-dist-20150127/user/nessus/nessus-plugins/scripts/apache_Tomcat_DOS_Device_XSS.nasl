#
# This script was written by Matt Moore <matt.moore@westpoint.ltd.uk>
#
# Script audit and contributions from Carmichael Security <http://www.carmichaelsecurity.com>
#      Erik Anderson <eanders@carmichaelsecurity.com>
#      Added BugtraqID
#      Also covers BugtraqID: 5193 (same Advisory ID#: wp-02-0008)
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(11042);
 script_version("$Revision: 1.8 $");
 script_bugtraq_id(5194);
 name["english"] = "Apache Tomcat DOS Device Name XSS";
 name["francais"] = "Apache Tomcat DOS Device Name XSS";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "Apache Tomcat is the servlet container that is used in the official Reference 
Implementation for the Java Servlet and JavaServer Pages technologies.

By making requests for DOS Device names it is possible to cause
Tomcat to throw an exception, allowing XSS attacks, e.g:

tomcat-server/COM2.IMG%20src='Javascript:alert(document.domain)'

(angle brackets omitted)

The exception also reveals the physical path of the Tomcat installation.

Solution: 

Upgrade to Apache Tomcat v4.1.3 beta or later.

References: 

www.westpoint.ltd.uk/advisories/wp-02-0008.txt (DOS Device XSS)
http://www.securiteam.com/windowsntfocus/5KP0L007FI.html (DOS device name
physical path disclosure).

Risk factor : Low";

 script_description(english:desc["english"]);
 
 summary["english"] = "Tests for Apache Tomcat DOS Device name XSS Bug";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2002 Matt Moore",
		francais:"Ce script est Copyright (C) 2002 Matt Moore");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "no404.nasl", "http_version.nasl", "cross_site_scripting.nasl");
 script_require_ports("Services/www", 80, 8080);
 script_require_keys("www/apache");
 exit(0);
}

# Check starts here

include("http_func.inc");

port = get_kb_item("Services/www");
if(!port)port = 8080;
if(!get_port_state(port)) exit(0);
if(get_kb_item(string("www/", port, "/generic_xss"))) exit(0);

req = http_get(item:"/COM2.<IMG%20SRC='JavaScript:alert(document.domain)'>", port:port);
soc = http_open_socket(port);
if(soc)
{ 
 send(socket:soc, data:req);
 r = http_recv(socket:soc);
 http_close_socket(soc);
 confirmed = string("alert(document.domain)"); 
 confirmed_too = string("java.io.FileNotFoundException");
 if ((confirmed >< r) && (confirmed_too >< r)) 	
 	security_hole(port);
}
