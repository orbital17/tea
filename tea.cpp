#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void strcopy(char * dest, char const * source, size_t size = 0)
{
	if (0 == size) size = strlen(source);
	while (size > 0) 
	{
		*dest = *source;
		++source;
		++dest;
		--size;
	}
	*dest = 0;
}

typedef enum {ENCRYPT, DECRYPT} Mode;

static int32_t calc_v(int32_t const * v, int32_t const * k, int n, int32_t sum)
{
    return ((v[1-n]<<4) + k[2*n]) ^ (v[1-n] + sum) ^ ((v[1-n]>>5) + k[2*n+1]);
}

void crypt(int32_t* v, int32_t const * k, Mode mode, unsigned int number_of_iteration)
{
    const int32_t delta = 0x9e3779b9;
    int orderVar;
    int32_t sum, multiplier;
    multiplier = (mode == ENCRYPT) ? 1 : -1;
    orderVar = (mode == ENCRYPT) ? 0 : 1;
    sum = (mode == ENCRYPT) ? delta : delta * number_of_iteration;
    for (int i=0; i < number_of_iteration; i++)
    {
        v[orderVar] += multiplier * calc_v(v, k, orderVar, sum);
        v[1 - orderVar] += multiplier * calc_v(v, k, 1 - orderVar, sum);
        sum += multiplier * delta;
    }
}


class IDataSource
{
    public:
        virtual int8_t getc() = 0;
        virtual bool hasNext() = 0;
};

class IDataTarget
{
    public:
    	//return no error indicator
        virtual bool putc(int8_t) = 0;
};

class FileSource : public IDataSource
{
	private:
        FILE * file;
        char nextChar; //we need this to make possible correct work of hasNext and not waste of speed simultaneously

    public:
        FileSource(char const* filename)
        {
            file = fopen(filename, "rb");
            nextChar = fgetc(file);
        }
        
        ~FileSource()
        {
        	fclose(file);
        }

        int8_t getc()
        {
        	int8_t result = (int8_t)nextChar;
        	nextChar = fgetc(file);
        	return result;
        }
        
        bool hasNext()
        {
        	return !feof(file);
        	
        }
};

class FileTarget : public IDataTarget
{
    private:
        FILE * file;

    public:
        FileTarget(char const* filename)
        {
            file = fopen(filename, "wb");
        }
        bool putc(int8_t c)
        {
            fputc(c, file);
            return !ferror(file);
        }
        
        ~FileTarget()
        {
        	fclose(file);
        }
        
        void rewindFile()
        {
        	rewind(file);
        }
};

class RandomSource : public IDataSource
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

class NullTarget : public IDataTarget
{
	public:
		bool putc(int8_t c)
		{
			return true;
		}
};

class StringSource : public IDataSource
{
	private:
		char * string;
		size_t size;
	public:		
		char * getValue()
		{
			return string;
		}
		
		size_t getSize()
		{
			return size;
		}
		
		void setValue(char const* s, size_t _size=0)
		{
			size = _size ? _size : strlen(s);
			string = new char[size + 1];
			strcopy(string, s, size);
		}
		StringSource(char const * s, size_t _size=0)
		{
			setValue(s, _size);
		}
		~StringSource()
		{
			delete[] string;
		}
		bool hasNext()
		{
			return (size > 0);
		}
		int8_t getc()
		{
			char r = *string;
			if (size > 0) {
				++string;
				--size;
			}
			return r;
		}
};

class StringTarget: public IDataTarget
{
	private:
		char * string;
		size_t size, iter;
	public:
		void reset()
		{
			string[0] = 0;
			iter=0;
		}
		
		StringTarget(size_t stringSize)
		{
			size = stringSize;
			string = new char[stringSize+1];
			reset();
		}
		
		~StringTarget()
		{
			delete[] string;
		}
		
		bool putc(int8_t c)
		{
			if (iter<size)
			{
				string[iter] = c;
				string[++iter] = 0;
				return true;
			}
			else
			{
				return false;
			}
		}
		char * getValue()
		{
			char * r = new char[size+1];
			strcopy(r, string, iter);
			return r;
		}
		
		size_t getSize()
		{
			return iter;
		}	
};

// returns number of bytes, that was readed
size_t readBytes(IDataSource * source, int8_t * target, size_t n)
{
	int i;
    for(i=0; i<n && source->hasNext(); i++)
    {
        target[i] = source->getc();
    }
    for(int j=i; j<n; j++)
    {
    	target[j] = 0;
    }
    return i;
}

//returns no error indicator
bool putBytes(IDataTarget *target, int8_t const* source, size_t n)
{
    bool result = true;
    for(int i=0; i<n && result; i++)
    {
        result = target->putc(source[i]);
    }
    return result;
}

//return number of bytes, that was readed during the last iteration
size_t engine(IDataSource * source, IDataTarget  * target, IDataSource * keySource, Mode mode, 
			size_t numberOfBytesInTheLastBlock = 8, size_t numberOfIterations = 32)
{
    int8_t v[8], key[16];
    bool doNotExitCycle = source->hasNext();
	readBytes(keySource, key, 16);
	size_t result;
    while(doNotExitCycle)
    {
        result = readBytes(source, v, 8);
        crypt((int32_t*)v, (int32_t*)key, mode, numberOfIterations);
        doNotExitCycle = source->hasNext();
        putBytes(target, v, doNotExitCycle? 8 : numberOfBytesInTheLastBlock);
    }
    return result;
}

void generateKeyToFile(char const* keyFile)
{
	const size_t keySize = 16;
	IDataSource* source = new RandomSource(keySize);
	IDataTarget* target = new FileTarget(keyFile);
	int8_t key[keySize];
	readBytes(source, key, keySize);
	putBytes(target, key, keySize);
	delete source;
	delete target;
}

void cryptFile(char const * source, char const * target, char const * keyFile, Mode mode)
{
	const size_t keySize = 16;
	int8_t key[keySize];	
	IDataSource* keySource = new FileSource(keyFile);
	readBytes(keySource, key, keySize);
	delete keySource;
	
	int8_t bytesInLastBlock = 8;
	FileSource* _source = new FileSource(source);
	FileTarget* _target = new FileTarget(target);

	
	if (mode == ENCRYPT) 
		putBytes(_target, &bytesInLastBlock, 1);//to allocate space at the beginning of file
	else
		readBytes(_source, &bytesInLastBlock, 1);
		
	bytesInLastBlock = engine(_source, _target, new StringSource((char*)key, keySize), mode, bytesInLastBlock);
	
	if (mode == ENCRYPT){
		_target->rewindFile();
		putBytes(_target, &bytesInLastBlock, 1);
	}

	delete _source;
	delete _target;
	
}

