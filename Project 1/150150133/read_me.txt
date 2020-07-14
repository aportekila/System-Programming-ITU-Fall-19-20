Module Authors:
	Bengisu Güresti	150150105
	Ufuk Demir		150170710
	Abdullah Akgül	150150133

In order to use the module with the easiest way, with the suggested way, you need to follow:
	chmod a+rx setup.sh // make executable the setup.sh
	./setup.sh // this need sudo su privileges and make you install the device easily

For the testing ioctl modules run the followings:
	For ioctl MMIND_REMAINING:
		./MMIND_REMAINING
			It will return the number of guesses the player B can.
			
	For ioctl MMIND_ENDGAME:
		./MMIND_ENDGAME
			It will ends the game.
			
	For ioctl MMIND_NEWGAME:
		./MMIND_NEWGAME <number>
			<number> is mmind_number, it need to be 4-digit other types get an error message.
				Please delete <> in order to not get an error message with 4-digit number
			It will starts a new game with new number
	
	For checking what would do the module after its full.
		We write a script named as quata_checker.sh
		In order to use it you should make it executable first by following command
			chmod a+rx quata_checker.sh
		Then you can try the quata_checkker with following command
			./quata_checker.sh
		It simply writes 256 guesses to the module.
		After run the quata_checker, you can try to write something to the module 
			but i will give an error.
