
# Also covers CVE-2000-0745, CVE-2001-0001 CVE-2001-0321 CVE-2001-0383 CVE-2001-0899 CVE-2001-0900 CVE-2001-1032
# BID : 1592,2422,2424,2431,2544,3106,3107,3114,3149,3361,3510,3567,3609,3889,3906,4302,4333,5476,5788,5796,5799,5953,6088,6244,6399,6400,6406,6409

if (description)
{
 script_id(11236);
 
  script_cve_id("CAN-2001-0292",
		"CAN-2001-0320",
		"CAN-2001-0854",
		"CAN-2001-0911",
		"CAN-2001-1025",
		"CAN-2002-0206",
		"CAN-2002-0483",
		"CAN-2002-1242");
 script_bugtraq_id(6446,6465,6503,6750,6887,6890,7031,7060,7078,7079);
 script_version ("$Revision: 1.11 $");
 script_name(english:"PHP-Nuke is installed on the remote host");
 desc["english"] = "
The remote host is running a copy of PHP-Nuke.

Given the insecurity history of this package, the Nessus
team recommands that you do not use it but
use something else instead, as security was clearly
not in the mind of the persons who wrote it.

The author of PHP-Nuke (Francisco Burzi) even started to rewrite
the program from scratch, given the huge number of vulnerabilities
(http://www.phpnuke.org/modules.php?name=News&file=article&sid=5640)

Solution : De-install this package and use something else
Risk factor : High";

 script_description(english:desc["english"]);
 script_summary(english:"Determines if PHP-Nuke is installed on the remote host");
 script_category(ACT_GATHER_INFO);
 script_family(english:"CGI abuses", francais:"Abus de CGI");
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison");
 script_dependencie("find_service.nes", "http_version.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}

include("http_func.inc");
include("http_keepalive.inc");

port = get_kb_item("Services/www");
if (!port) port = 80;


if(!get_port_state(port))exit(0);


function check(loc)
{
 req = http_get(item:string(loc), port:port);
 r = http_keepalive_send_recv(port:port, data:req);
 if( r == NULL ) exit(0);
 
 if(egrep(pattern:".*GENERATOR.*PHP-Nuke.*", string:r))return(1);
 else return(0);
}

dir[0] = "/";
dir[1] = "/nuke/";
dir[2] = "/demo/";
dir[3] = "/phpnuke/html/";
dir[4] = "/php_nuke/html/";
dir[5] = "/php/";
dir[6] = "/phpnew/";
dir[7] = "/nuke50/";
dir[8] = "/nuke60/";
dir[9] = "/nuke65/";

 for (i = 0; dir[i] ; i = i + 1)
 {
  z = dir[i];
  if(check(loc:z)){
  	security_hole(port);
	exit(1);
	}
 }
 
foreach dir (cgi_dirs())
{
if(check(loc:string(dir))){ security_hole(port); exit(0); }
}
