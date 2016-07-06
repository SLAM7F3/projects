===========================================================================
Feedback from Kevin O'Brien, Dan Dumanis & Ross Anderson, Thomas Callaghan
(G105), Tim Schreiner
===========================================================================
Last updated on 7/17/09; 7/19/09; 7/20/09; 7/21/09; 7/24/09; 7/26/09
===========================================================================

Priorities: P1(Low) P3(Med) P5(High)

===========================================================================
Changes still needing to be made to QTLOS
===========================================================================

Highest priority to lowest:

2.  Need an all-purpose reset button

3.  Limit total length of AVI/flash movies

4. When the 'Display time averaged results' button was clicked during a
raytracing, it showed a very high (-2147483648%) negative number for the
within % for all four of the occlusion percentage regions (0-25% etc). This
happened after I paused the video after a raytrace had finished. This has
occurred realtively frequently.

  a. When a new session was loaded and a new raytacing began, this number
remained.  I did not actually see any time averaged result (which I assume
is supposed to show up either next to or underneat it's label 'Time average
occlusion'.

5.  Dan is working with the laptop now, so I was unable to see exactly if I
could just change the permissions on the file executable so that it could
access things as though it were the superuser. I did seem to find a way to
consistently break the LOS program though. If you set the timestep to 1
Second (which is a possibility they would do if they wanted a more accurate
reading and could have it run all night I would think) then it records two
sets of data before it stops. If you click to display the percentages, then
it gives you that large number I mentioned before.

6. At times the flv player is out of view and cannot be interacted
with. When I started a recording and stopped it, it seemed to recording
what I was doing but when the video popped up, it was s video of the
previous video I captured, not the current one.

7.  Originally I forgot to hit update flightpath after drawing it
manually. Is there a feasible way to give the user a friendly reminder to
update the flightpath for it to show on the 3D viewer?

8. When update ROI is used with no input, a NaNZundefined appears on MGRS
text field (P1)


 3D Viewer

  1. I believe the time step option would be very confusing to a typical
user because I would assume they would imagine the data collection to be
continuous. Perhaps if there was a way to hide is option but mention it in
the help because it seems best for it to be left at the default unless
whoever is using it feels they need it to process more quickly and crudely
or slower but more accurately. (P1)

  2. It is stated that the pause button should be hit stated in the tooltip
on the Start Raytracing button, but tooltips are more ignored more often
than not I believe. Although it seems intuitive to hit pause to pause it,
they might believe it could mess up the computation. Also, when the pause
button is hit, it seems to delay for at least 10 seconds until it actually
pauses. It also holds the same occlusion percentages as the last test case
until the button is pressed and the user waits a moment. (P2)

 Session Mgt

  2. There may be a correlation between loading an old session and other
errors such as the -214...% one. However, those errors are not exclusive to
cases where a Session is loaded on top of another session.

  3. You cannot save over a file you have already saved (even if it is
saved as the same name). However, you can delete the old one. (P1)

Show alert when user is about to delete sessions from derby database...

*.  Dan's feedback:

*.  Documentation suggestion:  Suggest creating defaults session with
genuine param values...

*.  Grid lines on thin client

*.  Disable firebug before deployment

*.  Reset etc/resolv.conf to empty before deployment

*.  Change hardwired /home/cho/Desktop/movies_and_screen_shots/ to 
/home/LOS/Desktop/movies_and_screen_shots in LOSServer.cc !!!

*.  Change permissions on /usr/local/apache-tomcat/webapps/pathplanning and
/usr/local/apache-tomcat/webapps/pathplanning/movies to all write so that
los account on laptops can write movie.flv to these directories.

*.  Ross's feedback:

*.  Make raster images containing ray tracing info directly available to
end user's.  Wouldn't need to perform screen shots for high-level info.

*.  Use Symantec Ghost (owned by Norton) to perform DVD -> disk cloning in
the future.  

*.  Thomas Callaghan feedback:

*.  More "powerpoint-like" drawing/dragging capability within 2D map viewer
for flight paths, ROI bboxes, 

*.  Chris Brown's feedback:

*.  Keep I/O as simple as possible!

==========================================================================
Changes made to QTLOS program
==========================================================================

A.  Change current time HUD to "Time Average" when time-averaged results
are displayed.

B.  Somehow indicate when screen captures are being done and not.

C. When I loaded up a session, it still showed the old flight path that I
drew before I loaded the session. this was seen only on the 2D viewer, and
not the 3D viewer. Even though the 'circular' option is checked, the
general options appear below the General flightpath entry. If the radio
button is clicked to general then back to circular, this 'fixes' the
problem.

D. Also, using the 'clear all raytracing results button' did not clear any
of the timed average results, nor any of the data in the log. (P4)

E. Lower Left corner coords and Upper right corner coords can be any
corner. This label can be confusing if a user inputs the coordinates
(accidentally) so that the corners are improperly labeled (ie. under lower
Left corner they have (66,35) and on upper right corner they have (63,32)
This could potentially confuse results.(P3)

   a. Suggestion: either change labels to something like (Corner 1, Corner
2) or have the program check for this 'error' and alert the user that their
input corners are not as labeled.
 
F. Zoom in feature either unlimited or too high (P1)

G.  General path ---> Customized path

H. On "Draw Targets on 2D map" button, Draw sounds like an improper verb
for a point. I suggest replacing 'Draw' with 'Place'or 'Add' (P1)

I. I believe the tooltip for 'Update Targets' Button should say something
like "sets targets using coordinates below on the 2D viewer"  -->  Update =
Submit, broadcast

J.  Add thick client screen capture button to Movie Generation page

K.  Sensor params don't update if a flight path already exists...(e.g. left
lobe doesn't appear if double lobe already exists)

L.  Need some clear indicator as to when raytracing is finished.

M.  Display total flight distance and time after flight path is entered

O.  Enable user to label ground target points
