import curses, time
import os
import sys

#https://docs.python.org/2/library/curses.html
def get_key(stdscr):
	stdscr.nodelay(True);
	return stdscr.getch()

print(sys.argv)

fd = os.open(sys.argv[1], os.O_RDWR | os.O_NONBLOCK)
key = -1
while key != 27:
	
	key = curses.wrapper(get_key)
	if key != -1:
		os.write(fd, chr(key))
	v = os.read(fd, 1)
	if v:	
		print v
	#sys.stdout.flush()
	time.sleep(0.05)

os.close(fd)



