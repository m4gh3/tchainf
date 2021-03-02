#include <stdio.h>
#include <stdlib.h>

void def_buf_push(char **str, int *def_buf_allocated, int *def_buf_pos, char c )
{
	if( *def_buf_allocated <= *def_buf_pos + 1 )
	{
		*str = realloc(*str, *def_buf_allocated + 256 );
		*def_buf_allocated += 256;
	}
	(*str)[(*def_buf_pos)++] = c;
}

int main(int argc, char **argv )
{
	
	FILE *tproc, *source; //= fopen(argv[3], "r" );
	int mode; int def_buf_allocated = 0; int def_buf_pos = 0; char *def_buf=NULL;

	printf("@\nimport os\nimport sys\nsys.path.append(os.environ.get(\"HOME\")+\"/.tchainf\")\n");

	if(argc < 1 )
		goto err;

	switch(argv[1][0])
	{
		case 'f':
			mode = 0;
			break;
		case 'c':
			mode = 1;
			break;
		case 's':
			mode = 2;
			break;
		default:
		err:
			fprintf(stderr, "tchainf - tproc chain/filter - m4gh3 - utility to combine source code (with tproc macros) and tproc files\n"
				"usage:\n"
				"\ttchainf f tprocfile source0 source1 source2\tappends source processed in filter mode to tprocfile\n"
				"\ttchainf c tprocfile source0 source1 source2\tappends source processed in chain mode to tprocfile\n"
				"\ttchainf s source0 source1 source2\tconcatenates sources processed in script mode\n"
				"in this version extra arguments are ignored and will not cause error\n"
				"this version outputs to stdout only\n"
			);
			return 1;
	}

	if( (mode < 2 && argc < 3) || ( argc < 2 ) )
		goto err;

	if(mode < 2)
	{
		tproc = fopen(argv[2], "r" );

		for(int c; (c = fgetc(tproc))!=EOF; )
			putchar(c);
	}

	printf("\n@main\n");
	//if(!mode)
		printf("{gen_head}\n");

	for(int i=(mode==2?2:3); (i < argc) && ((source = fopen(argv[i], "r" )) != NULL); i++ )
	{
		if( source == NULL )
		{
			fprintf(stderr, "error: could not open %s\n", argv[i] );
			return 1;
		}
		for(int c,d,state=0; (c=fgetc(source)) != EOF; )
		{
			switch(state)
			{
				case 0:
				lcurl_check:
					switch(c)
					{
						case '{':
							d = fgetc(source);
							if(d == ':')
							{
								putchar('{');
								state=1;
							}
							else
							{
								c = d;
								if(mode)
								{
									putchar('\\');
									putchar('{');
								}
								goto lcurl_check; 	//don't fetch another char, don't change mode: check what
							       				//to do with c = d
							}
						break;
						case ':':
							if(mode)
							{
								putchar('\\');
								putchar(':');
							}
							break;
						case '}':
							if(mode)
							{
								putchar('\\');
								putchar('}');
							}
							break;
						case '"':
							if(mode)
								putchar('"');
							state = 2;
							break;
						case '@':
								def_buf_push(&def_buf, &def_buf_allocated, &def_buf_pos, '@' );
								state = 3;
							break;
						default:
							if(mode)
								putchar(c);
					}
					break;
				case 1:
					switch(c)
					{
						case '\\': //this is deceivingly useless: what happens if you have \}
							putchar('\\');
							putchar(fgetc(source));
							break;
						case '}':
							putchar('}');
							state=0;
						break;
						default:
							putchar(c);
					}
				break;
				case 2:
					switch(c)
					{
						case '\\': //this is deceivingly useless: what happens if you have \"
							if(mode)
							{
								putchar('\\');
								putchar(fgetc(source));
							}
						break;
						case '"':
							if(mode)
								putchar('"');
							state = 0;
						break;
						default:
							if(mode)
								putchar(c);
					}
				break;
				case 3:
					switch(c)
					{
						case '\\': //this is deceivingly useless: what happens if you have \"
							def_buf_push(&def_buf, &def_buf_allocated, &def_buf_pos, '\\' );
							def_buf_push(&def_buf, &def_buf_allocated, &def_buf_pos, fgetc(source) );
						break;
						case '@':
							state = 0;
							break;
						default:
							def_buf_push(&def_buf, &def_buf_allocated, &def_buf_pos, c );
					}
				break;
			}
		}
	}
	//if(!mode)
		printf("\n{gen_tail}\n");
	if(def_buf_allocated)
	{
		def_buf[def_buf_pos] = 0;	
		printf("\n%s", def_buf );
	}
}
