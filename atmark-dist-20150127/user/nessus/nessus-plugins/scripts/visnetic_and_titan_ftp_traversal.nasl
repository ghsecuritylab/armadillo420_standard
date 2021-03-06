#
# (C) Tenable Network Security 
#
# See: http://www.dhgroup.org/bugs/adv21.txt
#

if(description)
{
 script_id(11679);
 script_bugtraq_id(7718);
 script_version ("$Revision: 1.4 $");

 name["english"] = "VisNetic and Titan FTP Server traversal";

 script_name(english:name["english"]);
 desc = "
The remote FTP server is vulnerable to a flaw which allows users
to access files which are outside the FTP server root.

An attacker may break out of his FTP jail by issuing the command :

ftp> quote stat ../*

Solution : Contact your vendor for a patch
Risk factor : High";

 script_description(english:desc);

 summary["english"] = "Attempts to break out of the FTP root";
 script_summary(english:summary["english"]);

 script_category(ACT_GATHER_INFO);

 script_copyright(english:"This script is Copyright (C) 2003 Tenable Network Security",
francais:"Ce script est Copyright (C) 2003 Tenable Network Security");
 family["english"] = "FTP";
 family["francais"] = "FTP";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "ftp_anonymous.nasl");
 script_require_keys("ftp/login");
 script_require_ports("Services/ftp", 21);
 exit(0);
}


include("ftp_func.inc");

function dir()
{
 p = ftp_get_pasv_port(socket:soc);
 if(!p)exit(0);
 soc2 = open_sock_tcp(p, transport:get_port_transport(port));
 if(!soc2)return(0);
 ls = string("LIST .\r\n");
 send(socket:soc, data:ls);
 r = ftp_recv_line(socket:soc);
 if(egrep(pattern:"^150 ", string:r))
 {
  result = ftp_recv_listing(socket:soc2);
  close(soc2);
  r = ftp_recv_line(socket:soc);
  return(result);
 }
 return(0);
}


#
# The script code starts here
#

port = get_kb_item("Services/ftp");
if(!port)port = 21;
if(!get_port_state(port))exit(0);
soc = open_sock_tcp(port);
if(soc)
{
 login = get_kb_item("ftp/login");
 pass = get_kb_item("ftp/password");
 if(ftp_log_in(socket:soc, user:login, pass:pass))
 {
 send(socket:soc, data:string("stat ../*\r\n"));
 r = ftp_recv_line(socket:soc);
 listing1 = dir();
 if(!listing1)exit(0);

 send(socket:soc, data:string("CWD C:\r\n"));
 r = ftp_recv_line(socket:soc);
 listing2 = dir();
 if(!listing2)exit(0);

 close(soc);

 if(listing1 != listing2)security_hole(port);
 }
}
