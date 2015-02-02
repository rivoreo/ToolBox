extern int cat_main(int argc, char **argv);

int lsmod_main(int argc, char **argv) {
	char *cat_argv[] = { "cat", "/proc/modules", 0 };
	return cat_main(2, cat_argv);
}
