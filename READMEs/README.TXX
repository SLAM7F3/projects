========================================================================
Notes about running multiple jobs on internal (TX2500) & external (TX-X) 
LLGrids
========================================================================
Last updated on 1/24/12; 1/27/12; 4/12/12; 3/18/13
========================================================================

TX2500 notes:

If you machine doesn't have any naming service, you can choose one of the
following IP's:

Name:   txlogin.llgrid.ll.mit.edu
Address: 172.23.100.21
Name:   txlogin.llgrid.ll.mit.edu
Address: 172.23.100.22
Name:   txlogin.llgrid.ll.mit.edu
Address: 172.23.100.23
Name:   txlogin.llgrid.ll.mit.edu
Address: 172.23.100.24
Name:   txlogin.llgrid.ll.mit.edu
Address: 172.23.100.25

Username = PE16649
Password = LL e-mail password

========================================================================

TXX notes:

username = cho
IP = txx.llgrid.ll.mit.edu or len.llgrid.ll.mit.edu
# IP = 129.55.116.12

photosynth_shared subdirectory of cho can be seen directly by Noah

*.  Uploading files to external LLGrid (TX-X)

scp image.jpg cho@txx.llgrid.ll.mit.edu:photosynth_shared

*.  As of July 2011, TXX automatically times out windows which are open for
more than 24 hours.  So we must use the "nohup" = no hangup command in
order to prevent our big bundler jobs from dying when TXX closes a remote
login session.  We have therefore redefined our run_bundler and
run_siftgraph aliases in our .bashrc on TXX:

alias run_bundler='nohup /home/gridsan/snavely/local/bin/BundlerVocabGE.sh &'
alias run_siftgraph='nohup /home/gridsan/snavely/local/bin/BundlerSiftGraph.sh &'

*.  In order to see process tree on TXX, chant

		ps -e f

*.  State information:

qw = job is waiting/pending
r = job is running
dr = job is draining (trying to be deleted)

*.  In order to see how much RAM each individual node on TXX has, chant

		qhost | more

As of Jan 2012, TXX nodes have RAM which ranges from 2 - 6 GBytes in size.


*.  In order to see all jobs running owned by user cho

qstat

qstat -g c   (in order to see global job activity)

In order to list all jobs run by user cho within the past 5 days on TXX,
chant

	qacct -j -d 5 -o cho | more

In order to find out maximum RAM used by jobs named "*SIFT*" which ran on
TXX in past 5 days, chant

	 qacct -j -o cho -d 5 | egrep 'SIFT|maxvmem'


qstat -j 458

qacct -j 458  (Accounting file)

qsub -V -j y -cwd -b y -N RESIZE-Jul01_1132 convert -resize 2400x2400\> ././SDC12848.JPG images/SDC12848.jpg
Your job 490321 ("RESIZE-Jul01_1132") has been submitted
+ qsub-wait.sh RESIZE-Jul01_1132
qsub -sync y -b y -hold_jid RESIZE-Jul01_1132 -N SLEEP /bin/sleep 1
Your job 490322 ("SLEEP") has been submitted
Job 490322 exited with exit code 0.
+ date
Fri Jul  1 12:56:47 EDT 2011
+ echo '[- Extracting keypoints -]'
[- Extracting keypoints -]
+ rm -f sift.txt
+ ToSift.sh images


[cho@login-0-0 ~]$ qacct -j 490307
==============================================================
qname        normal              
hostname     len-f-14-38         
group        cho                 
owner        cho                 
project      NONE                
department   defaultdepartment   
jobname      RESIZE-Jul01_1132   
jobnumber    490307              
taskid       undefined
account      sge                 
priority     0                   
qsub_time    Fri Jul  1 12:57:30 2011
start_time   Wed Jun 29 09:53:59 2011
end_time     Wed Jun 29 09:54:03 2011
granted_pe   NONE                
slots        1                   
failed       0    
exit_status  0                   
ru_wallclock 4       


If failed = 0 and exit_status = 0, then this job successfully ran to
completion.



[ bjobs

bjobs -l 

In order to see all jobs running owned by all users, chant

bjobs -u all ]

*.  To see all jobs owned by user cho which were completed within past 1
day, chant

qacct -j -d 1 -o cho | more


*.  In order to kill all jobs owned by user cho, 

[ bkill -u cho 0 ] 

qdel '*'

qdel -f '*'  (-f for forced deletion)

After executing these delete commands, chant 

	qstat | wc -l

in order to see how many jobs still remain to be deleted.  On 1/24/12,
Chansup warned us NOT to start a new bundler job on TXX until all jobs from
a previous run have been successfully deleted!

*.  In order to see how busy TXX is at any time, chant
 
bqueues (look at normal queue information rather than pmatlab queue)

LLGrid_status (shows information only for pmatlab jobs)

*.  In order to see status of all the compute nodes, chant

bhosts

ok status = node ready to accept a new job

closed status = node already busy running jobs and cannot take any new ones

unavail status = node is offline


*. In order to check individual job status, chant

bhist -l jobID (with no brackets)



qsub -j y -cwd -N CONVERT-May04_1640 -o images/kermit010.ks /home/gridsan/snavely/code/Bundler-scripts/ConvertKeyfileToSameer.sh images/kermit010.jpg images/kermit010.key.gz 660.80306 


qsub -V -j y -cwd -b y -N SIFT-May04_1711 mogrify -format pgm images/kermit006.jpg; sift < images/kermit006.pgm > images/kermit006.key; rm images/kermit006.pgm; gzip images/kermit006.key



GenerateSkeletalScript.sh list.txt skeletal_input/components.txt buildpair_tracks_files buildpair-tracks-index.txt skeletal_output 9 > skeletal-script.txt

 GenerateSkeletalScript.sh list.txt skeletal_input/components.txt buildpair_tracks_files buildpair-tracks-index.txt skeletal_output 9

========================================================================

On 6/29/11, Chansup Byun told us that we can redirect standard output
generated by Noah's bundler scripts to /dev/null to avoid having huge
numbers of small files dumped into the bundler output directory

In qsub-exec.sh, add /dev/null as follows:

while read LINE
do
    if [ "$LINE" != "" ]; then
    echo qsub -V -j y -cwd -b y $QSUB_FLAGS "$LINE"
    sleep 0.02
    qsub -V -j y -cwd -b y $QSUB_FLAGS -o /dev/null "$LINE"
    fi
done

In qsub-wait.sh, add /dev/null as follows

echo "qsub -sync y -b y -hold_jid $1 -N SLEEP /bin/sleep 1"
qsub -j y -cwd -sync y -b y -hold_jid $1 -o /dev/null -N SLEEP /bin/sleep 1

========================================================================

On 7/28/11, we learned more useful commands from Chansup:

*.  In order to get touchy2 to login quickly to TXX, we need to comment out
last 2 lines in /etc/ssh/ssh_config

.
.
.
      HashKnownHosts yes
#     GSSAPIAuthentication yes
#     GSSAPIDelegateCredentials no

========================================================================

On 4/12/12, Bill Arcand taught us that the following command displays how
many inodes are available:

lfs df -i


========================================================================

On TX2500 from within /home/gridsan/PE16649/TXX/photosynth_shared

scp -r cho@129.55.116.18:photosynth_shared/Bombay .

scp -r cho@129.55.116.18:photosynth_shared/GrandCanyon .

scp -r cho@129.55.116.18:photosynth_shared/MIT32K_and_aerial .

scp -r cho@129.55.116.18:photosynth_shared/lighthawk .

scp -r cho@129.55.116.18:photosynth_shared/MIT2317 .

scp -r cho@129.55.116.18:photosynth_shared/MIT2317_orig .


rsync -v -r cho@129.55.116.18:photosynth_shared/kermit1 .

rsync -v -r cho@129.55.116.18:photosynth_shared/NewsWrap .

rsync -v -r cho@129.55.116.18:photosynth_shared/plume .

rsync -v -r cho@129.55.116.18:photosynth_shared/Raven .

rsync -v -r cho@129.55.116.18:photosynth_shared/HAFB .

rsync -v -r cho@129.55.116.18:photosynth_shared/MIT_aerial .

rsync -v -r cho@129.55.116.18:photosynth_shared/MIT32K_nodes1 .

rsync -v -r cho@129.55.116.18:photosynth_shared/MIT32K_nodes2 .

rsync -v -r cho@129.55.116.18:photosynth_shared/MIT32K_nodes3 .

rsync -v -r cho@129.55.116.18:photosynth_shared/MIT32K_nodes4 .

rsync -v -r cho@129.55.116.18:photosynth_shared/old_kermit .

