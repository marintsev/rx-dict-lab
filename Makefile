main:
	make -C ./Debug/ all

clean: 
	make -C ./Debug/ clean

mrproper:
	-@rm btree.db

run: lab
	./$<