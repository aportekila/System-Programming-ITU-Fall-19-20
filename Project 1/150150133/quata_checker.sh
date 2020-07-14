
MMIND_NUMBER=9999

for (( i=0 ; i<25; i++ ))
do
	./MMIND_NEWGAME $MMIND_NUMBER
	for (( j=0; j<10; j++ ))
	do
		echo "000$j" > /dev/mastermind
	done
done

./MMIND_NEWGAME $MMIND_NUMBER
for (( j=0; j<6; j++ ))
do
	echo "000$j" > /dev/mastermind
done

echo ""
echo "256 guesses is writed to the module"
echo "quata is full right now"
echo "you can check the error by triying to write a guess to the module !"
echo "good luck"
echo ""
