#ifndef __OSMCANVAS_H__
#define __OSMCANVAS_H__

#include "wxcanvas.h"
#include "osm.h"

class TileList;

class TileWay
    : public ListObject
{
    public:
        TileWay(OsmWay *way, TileList *allTiles, TileWay *next)
            : ListObject(next)
        {
            m_way = way;
            m_tiles = allTiles;
        }
        
        OsmWay *m_way; // the way to render
        TileList *m_tiles; // all tiles containing this way, to prevent multiple redraws
        
};

class OsmTile
    : public IdObject, public DRect
{
    public:
        OsmTile(unsigned id, double minLon, double minLat, double maxLon, double maxLat, OsmTile *next)
            : IdObject(id, next), DRect(minLon, minLat, maxLon, maxLat)
        {
            m_ways = NULL;
        }

        void AddWay(OsmWay *way, TileList *allTiles)
        {
            m_ways = new TileWay(way, allTiles, m_ways);
        }

        TileWay *m_ways;

};

class TileList
    : public ListObject
{
    public:
       TileList(OsmTile *t, TileList *m_next)
        : ListObject(m_next)
       {
           m_tile = t;
       }
       OsmTile *m_tile;
};

class OsmCanvas;

class TileRenderer
{
    public:
        TileRenderer(double minLon,double minLat, double maxLon, double maxLat, double dLon, double dLat)
        {
            m_tiles = NULL;
            unsigned id = 0;
            // build a list of empty tiles;
            for (double i = minLon; i < maxLon; i += dLon)
            {
                for (double j = minLat; j < maxLat; j += dLat)
                {
                    m_tiles = new OsmTile(id++, i, j, i+ dLon, j + dLat, m_tiles);
                }
            }
        }



        void AddWays(OsmWay *ways)
        {
            for (OsmWay *w = ways; w ; w = static_cast<OsmWay *>(w->m_next))
            {
                AddWay(w);
            }
        }

        void AddWay(OsmWay *way)
        {
            DRect bb = way->GetBB();
            
            TileList *allTiles = NULL;
            for (OsmTile *t = m_tiles; t; t = static_cast<OsmTile *>(t->m_next))
            {
                if (t->OverLaps(bb))
                {
                    allTiles = new TileList(t, allTiles);
                }
            }
            for (TileList *l = allTiles; l; l = static_cast<TileList *>(l->m_next))
            {
                l->m_tile->AddWay(way, allTiles);
            }
            
        }

        void RenderTiles(OsmCanvas *canvas, double lon, double lat, double w, double h);

    private:
        OsmTile *m_tiles;

        
};

class OsmCanvas
    : public Canvas
{
    public:
        OsmCanvas(wxWindow *parent, wxString const &fileName);
        void Render();

        // with explicit colours
        void RenderWay(OsmWay *w, wxColour lineColour, bool polygon = false, wxColour fillColour = wxColour(255,255,55));

        // with default colours
        void RenderWay(OsmWay *w);
        ~OsmCanvas();
    private:
        OsmData *m_data;
        DECLARE_EVENT_TABLE();

        void OnMouseWheel(wxMouseEvent &evt);
        void OnLeftDown(wxMouseEvent &evt);
        void OnLeftUp(wxMouseEvent &evt);
        void OnMouseMove(wxMouseEvent &evt);

        double m_scale;
        double m_xOffset, m_yOffset;

        int m_lastX, m_lastY;
        bool m_dragging;


        #define MAXPOLYCOUNT (1024*16)
        void StartPolygon()
        {
            m_polygonCount = 0;
            m_polygonVisible = true;
        }
        
        void AddPoint(int x, int y)
        {
            int w = m_backBuffer.GetWidth();
            int h = m_backBuffer.GetHeight();

        
            if (m_polygonCount < MAXPOLYCOUNT)
            {
                m_polygonPoints[m_polygonCount].x = x;
                m_polygonPoints[m_polygonCount].y = y;

                m_polygonCount++;

                if (x > 0 && x < w && y > 0 && y < h)
                {
                    m_polygonVisible = true;
                }
            }
            
        }
        
        void EndPolygon(wxDC *dc)
        {
            if (m_polygonCount)
                dc->DrawPolygon(m_polygonCount, m_polygonPoints);
        }

        wxPoint m_polygonPoints[MAXPOLYCOUNT];
        int m_polygonCount;
        bool m_polygonVisible;

        TileRenderer *m_tileRenderer;
        
};


#endif
