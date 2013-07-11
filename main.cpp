#include "tea.cpp"

//this is command line interface
//for realisation see "tea.cpp"
int main(int argc, char** argv)
{
	char const * encrypt = "-encrypt";
	char const * decrypt = "-decrypt";
	char const * genkey = "-genkey";
	char const * help = "-help";
	char const * h = "-h";
	char const * helpInfo = 
				"TEA Encryptor\n"
				"Usage: %s [option]\n"
				"Options:\n"
				"	-genkey FILE\n"
				"		generates a 16-bytes key and writes it in FILE\n"
				"	-encrypt|decrypt SOURCE_FILE DESTINATION_FILE KEY_FILE\n"
				"		encrypts or decrypts data in SOURCE_FILE using key from KEY_FILE\n"
				"		and writes it into DESTINATION_FILE\n"
				"	-h, -help\n"
				"		displays this help and exit\n";
	char const * error = "Error occured\n";
	char const * needMoreArguments = "Too few arguments\n";
	
	if (1 >= argc || 0 == strcmp(argv[1], help) || 0 == strcmp(argv[1], h)){
		printf(helpInfo, argv[0]);
		return 0;
	}
	if (0 == strcmp(argv[1], encrypt) || 0 == strcmp(argv[1], decrypt))
	{
		if (5 <= argc)
		{
			try{
				cryptFile(argv[2], argv[3], argv[4], 0 == strcmp(argv[1], encrypt) ? ENCRYPT : DECRYPT);
			} catch (...) { printf(error);}
		}
		else printf(needMoreArguments);
		return 0;
	}	
	if (0 == strcmp(argv[1], genkey))
	{
		if (3 <= argc)
		{
			try{
				generateKeyToFile(argv[2]);
			} catch (...) { printf(error);}
		}
		else
			printf(needMoreArguments);
		return 0;
	}
							
	
}
