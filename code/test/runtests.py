
import subprocess
import os 
from pprint import pprint




if __name__ == "__main__":

	build_path = '../build/'
	test_path = os.getcwd()
	test_files = os.listdir()
	os.chdir(build_path)
	# pprint(test_files)
	# pprint(os.listdir(os.getcwd()))
	subprocess.call('make clean;make',shell=True)
	subprocess.call('./nachos-userprog -rs 5 -x ./makethreads',shell=True)

    


