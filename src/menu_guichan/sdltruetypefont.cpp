                                                                     
                                                                      
                                                                      
                                                                     
                                                                    
                                                                   
                                                                  
  
                                                         
  
  
                             
                                    
  
                                        
  
                 
                                                                     
                                                                     
           
                                                                    
                                                                   
                                                                       
                                                                     
                                                                
                   
                                                                       
                                                                       
                                                
  
                                                                      
                                                                    
                                                                        
                                                                       
                                                                        
                                                                           
                                                                         
                                                                         
                                                                       
                                                                     
                                                               
   

  
                                                                
   

#include "sdltruetypefont.hpp"

#include "guichan/exception.hpp"
#include "guichan/image.hpp"
#include "guichan/graphics.hpp"
#include "guichan/sdl/sdlgraphics.hpp"

namespace gcn
{
    namespace contrib
    {
        SDLTrueTypeFont::SDLTrueTypeFont (const std::string& filename, int size)
        {
            mRowSpacing = 0;
            mGlyphSpacing = 0;
            mAntiAlias = true;        
            mFilename = filename;
            mFont = NULL;
        
            mFont = TTF_OpenFont(filename.c_str(), size);
        
            if (mFont == NULL)
            {
                throw GCN_EXCEPTION("SDLTrueTypeFont::SDLTrueTypeFont. "+std::string(TTF_GetError()));
            }
        }
    
        SDLTrueTypeFont::~SDLTrueTypeFont()
        {
            TTF_CloseFont(mFont);
        }
  
        int SDLTrueTypeFont::getWidth(const std::string& text) const
        {
            int w, h;
            TTF_SizeText(mFont, text.c_str(), &w, &h);
        
            return w;
        }

        int SDLTrueTypeFont::getHeight() const
        {
            return TTF_FontHeight(mFont) + mRowSpacing;
        }
    
        void SDLTrueTypeFont::drawString(gcn::Graphics* graphics, const std::string& text, const int x, const int y)
        {
            if (text == "")
            {
                return;
            }
        
            gcn::SDLGraphics *sdlGraphics = dynamic_cast<gcn::SDLGraphics *>(graphics);

            if (sdlGraphics == NULL)
            {
                throw GCN_EXCEPTION("SDLTrueTypeFont::drawString. Graphics object not an SDL graphics object!");
                return;
            }
        
                                                                                    
            int yoffset = getRowSpacing() / 2;
        
            Color col = sdlGraphics->getColor();

            SDL_Color sdlCol;
            sdlCol.b = col.b;
            sdlCol.r = col.r;
            sdlCol.g = col.g;

            SDL_Surface *textSurface;
            if (mAntiAlias)
            {
                textSurface = TTF_RenderText_Blended(mFont, text.c_str(), sdlCol);
            }
            else
            {
                textSurface = TTF_RenderText_Solid(mFont, text.c_str(), sdlCol);
            }
        
            SDL_Rect dst, src;
            dst.x = x;
            dst.y = y + yoffset;
            src.w = textSurface->w;
            src.h = textSurface->h;
            src.x = 0;
            src.y = 0;
        
            sdlGraphics->drawSDLSurface(textSurface, src, dst);
            SDL_FreeSurface(textSurface);        
        }
    
        void SDLTrueTypeFont::setRowSpacing(int spacing)
        {
            mRowSpacing = spacing;
        }

        int SDLTrueTypeFont::getRowSpacing()
        {
            return mRowSpacing;
        }
    
        void SDLTrueTypeFont::setGlyphSpacing(int spacing)
        {
            mGlyphSpacing = spacing;
        }
    
        int SDLTrueTypeFont::getGlyphSpacing()
        {
            return mGlyphSpacing;
        }

        void SDLTrueTypeFont::setAntiAlias(bool antiAlias)
        {
            mAntiAlias = antiAlias;
        }

        bool SDLTrueTypeFont::isAntiAlias()
        {
            return mAntiAlias;        
        }    
    }
}

