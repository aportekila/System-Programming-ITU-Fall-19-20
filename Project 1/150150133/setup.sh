

echo ""
echo "Module Writers"
echo -e "\tBengisu Guresti"
echo -e "\tUfuk Demir"
echo -e "\tAbdullah Akgul"
echo ""


MMIND_NUMBER=4070

read -n1 -p "Do you want to enter mmind_number? [y,n] " doit 
echo ""
case $doit in  
  y|Y) 
	echo "Select a mmind_number ";
	read MMIND_NUMBER;
	echo "selected mmind_number = $MMIND_NUMBER";; 
  n|N|*) 
	echo "default mmind_number = $MMIND_NUMBER";; 
esac



NODE="/dev/mastermind"

if test -e "$NODE"; 
then
	echo "Node $NODE is deleted..."
	sudo rm -r /dev/mastermind
else
	echo "There is no node $NODE..."
fi

MODULE="mastermind"

if lsmod | grep "$MODULE" &> /dev/null ; 
then
  echo "Module $MODULE is loaded before!"
  sudo rmmod mastermind
  echo "Module $MODULE is deleted!"
else
  echo "$MODULE is not loaded before!"
fi

echo ""
echo "make command is executing..."

sudo su <<EOF
	make
EOF

if [ $? -eq 0 ]
then
  echo "Successfully made make"
else
  echo "Could not compile" >&2
  exit 1
fi

sudo su <<EOF
	insmod mastermind.ko mmind_number="$MMIND_NUMBER"
EOF

if [ $? -eq 0 ]
then
  echo "Module $MODULE is installed successfully with mmind_number:$MMIND_NUMBER ..."
else
  echo "Could not install module $MODULE" >&2
  exit 1
fi

MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)
MINOR=0
echo "Module $MODULE major number is $MAJOR"



sudo su <<EOF
	mknod $NODE c $MAJOR $MINOR
EOF

if [ $? -eq 0 ]
then
  echo "Node $Node is created successfully with $MAJOR $MINOR ..."
else
  echo "Could not create node $NODE" >&2
  exit 1
fi


echo "setup is done you can try our module right now"
echo "have fun!!"
echo ""
echo "Note: you can check the debug messages with"
echo -e "\tdmesg | grep mmind:"
