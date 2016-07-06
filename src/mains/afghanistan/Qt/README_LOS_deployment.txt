===========================================================================
Final alterations to make to LOS laptops prior to shipping out of LL
===========================================================================
Last updated on 7/27/09; 7/28/09; 7/30/09
===========================================================================

*.  Disable firebug before deployment

*.  Reset /etc/resolv.conf to empty before deployment

*.  Change hardwired /home/cho/Desktop/movies_and_screen_shots/ to 
/home/LOS/Desktop/movies_and_screen_shots in LOSServer.cc !!!

*.  Change permissions on /usr/local/apache-tomcat/webapps/pathplanning and
/usr/local/apache-tomcat/webapps/pathplanning/movies to all write so that
los account on laptops can write movie.flv to these directories.

  chmod --recursive a+w /usr/local/apache-tomcat/webapps/pathplanning/movies/

*.  Turn off navigation & bookmarks toolbars in firefox for LOS account.

*.  Purge any movies or screen shots in both the movies_and_screen_shots
folder as well as the trash can.

*.  On 7/30/09, we experienced a near heart attack when we discovered that
thin-to-thick client communication on the newly cloned laptop failed for
the los user account.  On the other hand, QTLOS worked fine for the cho
user on the laptop.  After lots of help from Delsey, we remembered that
firefox on the laptops must always believe it's online even when it's
really offline.  So the Work Offline box must be unchecked!  In order to
force firefox to keep this box unchecked, chant about:config in the main
firefox menu.  Then search for toolkit.networkmanager.disable and toggle
this boolean to true.

QTLOS seems to work fine after for the LOS account user on the laptop after
we made this critical change.


