#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

int
main (int argc, char *argv[])
{

  int ret, fds, fdr;
  size_t outlen;

  fds = open ("/dev/clarona-rdseed", O_RDONLY);
  fdr = open ("/dev/clarona-rdrand", O_RDONLY);

  if (argc == 2)
    outlen = atoi (argv[1]);
  else
    outlen = 3;

  unsigned char *out =
    (unsigned char *) malloc (outlen * sizeof (unsigned char));

  if (fds >= 0)
    {
      ret = read (fds, out, outlen);
      if (ret < 0)
	{
	  perror ("Failed to read from rdseed");
	}
      else
	{
	  printf ("rdseed returned: ");
	  for (int i = 0; i < ret; i++)
	    printf ("0x%02x ", out[i]);
	  printf ("\n");
	}
    }
  else
    {
      printf ("rdseed is not accessible\n");
    }

  if (fdr >= 0)
    {
      ret = read (fdr, out, outlen);
      if (ret < 0)
	{
	  perror ("Failed to read from rdrand");
	}
      else
	{
	  printf ("rdrand returned: ");
	  for (int i = 0; i < ret; i++)
	    printf ("0x%02x ", out[i]);
	  printf ("\n");
	}
    }
  else
    {
      printf ("rdrand is not accessible\n");
    }

  free (out);
  if (fdr >= 0)
    close (fdr);
  if (fds >= 0)
    close (fds);

  return 0;
}
