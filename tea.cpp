#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma warning

void strcopy(char * dest, char * source)
{
	while ( *dest = *source) 
	{
		
		source++;
		dest++;
	}
	*dest = *source;
}

typedef enum {ENCRYPT, DECRYPT} Mode;

static int32_t calc_v(int32_t const * v, int32_t const * k, int n, int32_t sum)
{
    return ((v[1-n]<<4) + k[2*n]) ^ (v[1-n] + sum) ^ ((v[1-n]>>5) + k[2*n+1]);
}

void crypt(int32_t* v, int32_t const * k, Mode mode, unsigned int number_of_iteration)
{
    const int32_t delta = 0x9e3779b9;
    int orderVar, i;
    int32_t sum, multiplier;
    multiplier = (mode == ENCRYPT) ? 1 : -1;
    orderVar = (mode == ENCRYPT) ? 0 : 1;
    sum = (mode == ENCRYPT) ? delta : delta * number_of_iteration;
    for (i=0; i < number_of_iteration; i++)
    {
        v[orderVar] += multiplier * calc_v(v, k, orderVar, sum);
        v[1 - orderVar] += multiplier * calc_v(v, k, 1 - orderVar, sum);
        sum += multiplier * delta;
    }
}


class DataSource
{
    public:
        virtual int8_t getc() = 0;
        virtual bool hasNext() = 0;
};

class DataTarget
{
    public:
        virtual int putc(int8_t) = 0;
};

class FileSource : public DataSource
{
        FILE * file;

    public:
        FileSource(char * filename)
        {
            file = fopen(filename, "r");
        }

        int8_t getc()
        {
            return (int8_t)fgetc(file) ;
        }
        
        bool hasNext()
        {
        	char a = fgetc(file);
        	ungetc(a, file);
        	return (a!=EOF);
        	
        }
};

class FileTarget : public DataTarget
{
    private:
        FILE * file;

    public:
        FileTarget(char * filename)
        {
            file = fopen(filename, "w");
        }
        int putc(int8_t c)
        {
            return fputc((char)c, file);
        }
};

class RandomSource : public DataSource
{
	private:
		int leftData;
    public:
        RandomSource(int dataSize)
        {
            srand(time(NULL));
            leftData = dataSize;
        }
        
        int8_t getc()
        {
        	--leftData;
            return (int8_t)rand()%256;
        }
        
        bool hasNext()
        {
        	return (leftData > 0);
        }
};

class NullTarget : public DataTarget
{
	public:
		int putc(int8_t c)
		{
			return 1;
		}
};

class StringSource : public DataSource
{
	private:
		char * string;
	public:		
		char * getValue()
		{
			return string;
		}
		
		void setValue(char * s)
		{
			string = s;
		}
		StringSource(char * s)
		{
			setValue(s);
		}
		bool hasNext()
		{
			return (0!=(*string));
		}
		int8_t getc()
		{
			char r = *string;
			if (r) ++string;
			return r;
		}
};

class StringTarget: public DataTarget
{
	private:
		char * string;
		size_t size, i;
	public:
		void reset()
		{
			string[0] = 0;
			i=0;
		}
		
		StringTarget(size_t stringSize)
		{
			size = stringSize;
			string = (char*)malloc(stringSize+1);
			reset();
		}
		
		int putc(int8_t c)
		{
			if (i<size)
			{
				string[i] = c;
				string[++i] = 0;
				return 1;
			}
			else
			{
				return 0;
			}
		}
		char * getValue()
		{
			char * r;
			r = (char*) malloc(size+1);
			strcopy(r, string);
			return r;
		}	
};

void readBytes(DataSource * source, int8_t * target, size_t n)
{
    for(int i=0; i<n; i++)
    {
        target[i] = (source->hasNext() ? source->getc() : 0);
    }
    
}

int putBytes(DataTarget *target, int8_t* source, size_t n)
{
    int r = 1;
    for(int i=0; i<n && r; i++)
    {
        r = target->putc(source[i]);
    }
    return r;
}

void engine(DataSource * source, DataTarget  * target, DataSource * keySource, Mode mode, unsigned int number_of_iteration = 32)
{
    int8_t v[8], key[16];
    int j;
    bool doNotExitCycle = source->hasNext();
	readBytes(keySource, key, 16);
    while(doNotExitCycle)
    {
        readBytes(source, v, 8);
        crypt((int32_t*)v, (int32_t*)key, mode, number_of_iteration);
        putBytes(target, v, 8);
        doNotExitCycle = source->hasNext();
    }
}



