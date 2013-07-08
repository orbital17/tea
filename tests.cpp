#include<assert.h>
#include "tea.cpp"




void StringSourceIsWorking()
{
	char * text = "abcdeabcdeabcdeabcde";
	StringSource* a = new StringSource(text);
	assert(strcmp(text, a->getValue())==0 && text[0] == a->getc());
}

void StringTargetIsWorking()
{
	char * text = "abcdeabcdeabcdeabcde";
    StringSource*a = new StringSource(text);
    StringTarget*b = new StringTarget(strlen(text));
    while (a->hasNext())
    {
    	b->putc(a->getc());
    }
	assert(strcmp(text, b->getValue()) == 0);
}

void RandomSourceIsWorking()
{
	int n = 30;
	RandomSource* r = new RandomSource(n);
	for(int i = 0; i<n; i++)
	{
		assert(r->hasNext());
		r->getc();
	}
	assert(!r->hasNext());
}

void readBytesIsWorking()
{
	char * text = "jljkljkljjk";
	StringSource* a = new StringSource(text);
	int8_t *s;
	s = (int8_t*)malloc(strlen(text)+1);
	s[strlen(text)]=0;
	readBytes(a, s, strlen(text));
	assert(0 == strcmp((char*)s, text));
}

void putBytesIsWorking()
{
	char * text = "jskfldghdsufdibg";
	StringTarget * t = new StringTarget(strlen(text));
	putBytes(t, (int8_t*)text, strlen(text) );
	assert( 0 == strcmp(text, t->getValue()) );
	
}

void crypt_decryptOfEncryptIsTheSame()
{
	char * text = "abcdefjh";
	StringSource* vs = new StringSource(text);
	StringSource* ks = new StringSource("afdfdsalfjaksjkl");
	int8_t v[8];
	int8_t k[16];
	readBytes(vs, v, 8);
	readBytes(ks, k, 16);
	crypt((int32_t*)v, (int32_t*)k, ENCRYPT, 32);
	crypt((int32_t*)v, (int32_t*)k, DECRYPT, 32);
	StringTarget* vt = new StringTarget(8);
	putBytes(vt, v, 8);
	assert( 0 == strcmp(text, vt->getValue()) );
	
}

void engine_decryptOfEncryptIsTheSame()
{
	char* key = "jfkldsajfklaskfjakljf";
	char* text = "afasdfadsf";
	StringSource* a = new StringSource(text);
	StringTarget* b = new StringTarget(strlen(text)+7);
	StringSource* keySource = new StringSource(key);
	engine(a, b, keySource, ENCRYPT);
	a->setValue(b->getValue());
	keySource->setValue(key);
	b->reset();
	engine(a, b, keySource, DECRYPT);
//	printf("%s\n", b->getValue());
	assert( 0 == strcmp(text, b->getValue()) );
	
}

void filesTesting(Mode mode)
{
	char * in = "in.txt";
	char * out = "out.txt";
	FileSource * a = new FileSource(mode == ENCRYPT? in : out);
	FileTarget * b = new FileTarget(mode == DECRYPT? in : out);
	char* key = "jfkldsajfklaskfjakljf";
	StringSource* keySource = new StringSource(key);
	engine(a, b, keySource, mode);
}


int main()
{
	StringSourceIsWorking();
	StringTargetIsWorking();
	RandomSourceIsWorking();
	readBytesIsWorking();
	putBytesIsWorking();
	crypt_decryptOfEncryptIsTheSame();
	engine_decryptOfEncryptIsTheSame();
//	filesTesting(ENCRYPT);
}
