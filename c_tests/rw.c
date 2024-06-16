main()
{	
	int fdr, fdw, n;
	char buf[256];
	fdr = open("rw.c", 0);
	fdw = open("output.txt", 1);
	n = read(fdr, buf, sizeof(buf));
	write(fdw, buf, n);
	close(fdr);
	close(fdw);	
	return 0;
}
