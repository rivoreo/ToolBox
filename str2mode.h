#define STR2MODE(S) ({						\
		const char *s = (S);				\
		mode_t r = 0;					\
		while(*s) {					\
			if(*s >= '0' && *s <= '7') {		\
				r = (r << 3) | (*s - '0');	\
			} else {				\
				r = (mode_t)-1;			\
				break;				\
			}					\
			s++;					\
		}						\
		r & S_IFMT ? (mode_t)-1 : r;			\
	})
