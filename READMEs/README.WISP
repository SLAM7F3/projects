===========================================================================
WISP notes
===========================================================================
Last updated on 4/11/11; 4/15/11
===========================================================================

*. In order to connect to G99's public WISP imagery directory

   a.  Click Places -> Connect to Server

   b.  Change Service type to Windows share

   c.  Enter visionsystems for Server

   d.  Enter wisp for share

   e.  Enter pe16649 for User Name

   f.  Enter mitll for Domain Name


*.  In order to connect to G106 Tech Challenge ALIRT directory

   a.  Click Places -> Connect to Server

   b.  Change Service type to Windows share

   c.  Enter detox.llan.ll.mit.edu for server

   d.  Enter data1 for share

   e.  Enter pe16649 for User Name

   f.  Enter mitll for Domain Name


*.  In order to mount DETOX directory to /mnt/detox, chant

sudo mount -t smbfs -o username=pe16649,password=@Kermitd1 //detox.llan.ll.mit.edu/data1 /mnt/detox/
