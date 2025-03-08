//---------------------------------------------------------------------------

#ifndef tilesH
#define tilesH
//---------------------------------------------------------------------------
#include <vcl.h>
//---------------------------------------------------------------------------

struct NullOption
{       };
template<typename T>
class Optional
{
	T* o;
	public:
	~Optional()
	{
		if(o != nullptr)
			delete o;
        o = nullptr;
    }
	Optional(NullOption)
	{
		o = nullptr;
	}
	Optional(T * obj)
	{
		o = obj;
	}
	Optional(T&& obj)
	{
		std::allocator<T> a;
		o = a.allocate(1);
        new (o) T(std::forward<T>(obj));
    }
	bool isEmpty()   {   return o == nullptr;   }
    T& operator()()     {   return &o;      }
};
class Context
{
	public:
	virtual ~Context()
	{       }
    virtual void pipeToStdout() {}
	virtual void runCommand(String str) = 0;
	virtual bool isEnded() const = 0;
	virtual Optional<String> waitTillEnd() = 0;
	virtual Optional<String> getLastError() = 0;
};
class Tile
{
	String image;
	String command;
	public:
    void * component;
	Tile(String img, String cmd) :
		image(img), command(cmd), component(nullptr)
    {       }
	String getImage() const { return image; }
	void setImage(String str) { if(str != "")   image = str;    }
	String getCommand() const { return command; }
	void setCommand(String str) { if(str != "")   command = str;    }
	String serialize(void) const;
	void deserialize(const String str);
	Context * run(String path);
};
//---------------------------------------------------------------------------
#endif
