# CMPT300As2
* network.c, receiveMessage is a Producer, sendMessage() is a Consumer.
* screen.c, write_stdout is a Consumer, keyboard.c read_stdin is a Producer.
* List1 -> Producer: keyboard.c ; Consumer -> network.c/sendMessage()
* List2 -> Producer: network.c/receiveMessage() ; Consumer: screen.c