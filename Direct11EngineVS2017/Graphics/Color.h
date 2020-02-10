#pragma once
typedef unsigned char BYTE;

class Color
{
public:
	Color();
	Color(unsigned int val);
	Color(BYTE r, BYTE g, BYTE b);
	Color(BYTE r, BYTE g, BYTE b, BYTE a);
	Color(const Color& src);

	Color& operator = (const Color& src);
	bool operator == (const Color& rhs) const;
	bool operator != (const Color& rhs) const;

	BYTE GetR() const;
	void SetR(BYTE r);

	BYTE GetG() const;
	void SetG(BYTE g);

	BYTE GetB() const;
	void SetB(BYTE b);

	BYTE GetA() const;
	void SetA(BYTE a);

private:
	union
	{
		BYTE rgba[4];
		unsigned int color;
	};
};

namespace Hw3d_Colors
{
	const Color UnloadedTextureColor(100, 100, 100);
	const Color UnhandledTextureColor(250, 0, 0);
}

