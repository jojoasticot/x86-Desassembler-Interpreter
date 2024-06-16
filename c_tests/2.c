main() {
    putchar('a');
}

putchar(ch) char ch; {
	write(1, &ch, 1);
	return ch;
}
