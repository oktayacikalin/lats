/*
 * Thanks to gqview :) filelist.c text conversation utils
 * It is really great! ...the best one I could find :)
 * slightly modified for output like b, kb, mb, gb, tb
 * multiple gives me an index -> 0 = b, 1 = kb ... 4 = tb
*/

#include <gnome.h>

#include "importedfuncs.h"

gchar *text_from_size(int size)
{
        gchar *a, *b;
        gchar *s, *d;
        gint l, n, i;
		int submit;
		static gchar submit_string[2048];
		int multiple = 0;

        /* what I would like to use is printf("%'d", size)
         * BUT: not supported on every libc :(
         */

        a = g_strdup_printf("%d", size);
        l = strlen(a);
        n = (l - 1)/ 3;
        if (n < 1) return a;

        b = g_new(gchar, l + n + 1);

        s = a;
        d = b;
        i = l - n * 3;
        while (*s != '\0')
                {
                if (i < 1)
                        {
                        i = 3;
                        *d = ',';
                        d++;
                        }

                *d = *s;
                s++;
                d++;
                i--;
                }
        *d = '\0';

        g_free(a);
				
		submit = size ;
		//printf("%d %d\n", submit, b);
		while ( submit > 1024 && multiple < 5 )
		{
			submit = submit / 1024 ;
			//printf("submit verkleinern auf %d\n",submit);
			multiple++ ;
			//printd("multiple jetzt bei "); printd(multiple);
		}
		
		if ( multiple <= 0 ) sprintf( submit_string, "%dbytes", submit );
		if ( multiple == 1 ) sprintf( submit_string, "%dkb", submit );
		if ( multiple == 2 ) sprintf( submit_string, "%dmb", submit );
		if ( multiple == 3 ) sprintf( submit_string, "%dgb", submit );
		if ( multiple >= 4 ) sprintf( submit_string, "%dtb", submit );
		
		//printd(submit_string); printd("\n");
		return submit_string ;
}


gchar *text_from_time(time_t t)
{
        static gchar ret[64];
        struct tm *btime;

        btime = localtime(&t);

        /* the %x warning about 2 digit years is not an error */
        if (strftime(ret, sizeof(ret), /*"%x %H:%M"*/ "%c", btime) < 1) return "";

        return ret;
}

gchar *text_from_var(int size)
{
	static gchar submit_string[2048] ;
	
	sprintf( submit_string, "%d", size );
	
	return submit_string ;
}
