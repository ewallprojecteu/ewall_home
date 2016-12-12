Contents:

Accelerometer: Stand-alone version for reading accelerometer data from files, can also be run on Windows

Arduino_uC_code: Code for the Arduino Boards from UPB (arduino uno - environmental sensors & arduino lilypad USB for the accelerometer) 

Device Gateway: Middleware for connecting sensors, see the Readme.txt therein for installation. Also contains the perceptual components that process the sensor signals.

timeserv.cfg: configuration file specifying the time server needed for time synchronization


#Useful commands for managing the project:

#To delete the backup files:
find -name "*~" -delete

#Useful git commands
git clone http://user:pass@serv2.radio.pub.ro/gitlab/wp3group/wp3project.git
git status
git diff
git diff --staged
git add .
git commit -m "DGW"

#to combine add and commit, use
git commit -a -m "commit log"

#undo last commit
git reset --soft HEAD~1

#reset a file to last commit (http://stackoverflow.com/a/1817774)
#if not committed or added to index
git checkout filename
#if added to the index but not commited
git reset HEAD filename
git checkout -- filename
#if committed
git checkout origin/master filename

#stash commands
git stash list
git stash save "test changes"
git diff stash@{0} master

#Gitlab update:
git pull
git push -u origin master

#Clean all changes (http://gitready.com/beginner/2009/01/16/cleaning-up-untracked-files.html)
git clean -d -i

#configure name-email
git config user.name "Yourname"
git config user.email "user@server.com"

#check configured name-email
git config user.name
git config user.email

