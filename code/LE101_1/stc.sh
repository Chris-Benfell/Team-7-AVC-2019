 #!/bin/sh   
echo Deleting old c files 
ssh -x pi@10.140.197.139 'cd /home/pi/E101/Try1; rm *.cpp'   
#rsync -e 'ssh -ax' --include '.cpp' --exclude '*' -av /home/arthur/ENGR101/2017/Try1 pi@10.140.197.139:/home/pi/E101 
echo Transfering files to PI.. 
rsync -e 'ssh -ax' -av /home/arthur/ENGR101/2017/Try1 pi@10.140.197.139:/home/pi/E101 
echo Compiling and executing..
ssh -x pi@10.140.197.139 'cd /home/pi/E101/Try1; make;./start'
