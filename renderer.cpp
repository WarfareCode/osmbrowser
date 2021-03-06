// this file is part of osmbrowser
// copyright Martijn Versteegh
// osmbrowser is licenced under the gpl v3
#include "renderer.h"

#include <wx/image.h>

void RendererWxBitmap::DrawCenteredText(char const *s, double x, double y, double angle, int r, int g, int b, int a,  int layer)
{
	// not implemented yet
}

void RendererWxBitmap::Setup(wxBitmap *bitmap)
{
	m_output = bitmap;

	int w = bitmap->GetWidth();
	int h = bitmap->GetHeight();
	m_outputWidth = w;
	m_outputHeight = h;
	for (int i = 0; i < m_numLayers; i++)
	{
		m_layer[i].Create(w, h);
		m_dc[i].SelectObject(m_layer[i]);
		if (i)
		{
			m_dc[i].SetBackground(wxBrush(m_maskColor));
		}
	}
}

void RendererWxBitmap::DrawPolygon()
{
	ScalePoints();
	m_dc[m_curLayer].SetPen(m_pen);

	m_dc[m_curLayer].SetBrush(m_brush);
	m_dc[m_curLayer].DrawPolygon(m_numPoints, m_wxPoints);
}

void RendererWxBitmap::DrawLine()
{
	ScalePoints();
	m_dc[m_curLayer].SetPen(m_pen);
	m_dc[m_curLayer].DrawLines(m_numPoints, m_wxPoints);
}

void RendererWxBitmap::BlitWithTransparency(wxBitmap *from, wxImage *to)
{

	wxImage f = from->ConvertToImage();
	int w = f.GetWidth();
	int h = f.GetHeight();

	int mr = m_maskColor.Red();
	int mg = m_maskColor.Green();
	int mb = m_maskColor.Blue();

	for (int y = 0; y < h ; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int r = f.GetRed(x, y);
			int g = f.GetGreen(x, y);
			int b = f.GetBlue(x, y);

			if (r != mr || g != mg || b != mb)
			{
				to->SetRGB(x, y, r, g, b);
			}
		}
	}

}

void RendererWxBitmap::Commit()
{
	wxMemoryDC to;
	to.SelectObject(*m_output);


	wxImage composite = m_layer[0].ConvertToImage();

	for (int i = 1; i < m_numLayers; i++)
	{
		BlitWithTransparency(m_layer + i, &composite);
	}

	wxBitmap c(composite);

	to.DrawBitmap(c,0,0);

}

