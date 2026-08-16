                                                                     
                                                                      
                                                                      
                                                                     
                                                                    
                                                                   
                                                                  
  
                                                         
  
  
                             
                                    
  
                                        
  
                 
                                                                     
                                                                     
           
                                                                    
                                                                   
                                                                       
                                                                     
                                                                
                   
                                                                       
                                                                       
                                                
  
                                                                      
                                                                    
                                                                        
                                                                       
                                                                        
                                                                           
                                                                         
                                                                         
                                                                       
                                                                     
                                                               
   

#ifndef GCN_CONTRIB_SDLTRUETYPEFONT_HPP
#define GCN_CONTRIB_SDLTRUETYPEFONT_HPP

#include <map>
#include <string>

#include <SDL/SDL_ttf.h>

#include "guichan/font.hpp"
#include "guichan/platform.hpp"

namespace gcn
{
    class Graphics;
    namespace contrib
    {
        
           
                                                                                 
                                               
          
                                                                          
                                                                         
                          
          
                                  
                               
           
        class GCN_EXTENSION_DECLSPEC SDLTrueTypeFont: public Font
        {
        public:

               
                           
                   
                                                                  
                                                          
               
            SDLTrueTypeFont (const std::string& filename, int size);

               
                          
               
            virtual ~SDLTrueTypeFont();
      
               
                                                                            
                                           
              
                                                    
               
            virtual void setRowSpacing (int spacing);
      
               
                                                       
              
                                   
               
            virtual int getRowSpacing();
      
               
                                                                               
                                           
              
                                                    
               
            virtual void setGlyphSpacing(int spacing);

               
                                                          
              
                                   
               
            virtual int getGlyphSpacing();
      
               
                                              
              
                                                               
               
            virtual void setAntiAlias(bool antiAlias);
      
               
                                               
              
                                                     
               
            virtual bool isAntiAlias();
      
        
                                  
      
            virtual int getWidth(const std::string& text) const;
      
            virtual int getHeight() const;        
      
            virtual void drawString(Graphics* graphics, const std::string& text, int x, int y);
      
        protected:
            TTF_Font *mFont;
      
            int mHeight;
            int mGlyphSpacing;
            int mRowSpacing;
      
            std::string mFilename;
            bool mAntiAlias;      
        }; 
    }
}

#endif

