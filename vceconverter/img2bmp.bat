:: This bat file was auto generated by vceconverter  because it didn't exist in the folder.
:: Change the Image Magick path if you need.
:: 
:: 
@echo off
"%PROGRAMFILES(X86)%\ImageMagick-7.0.8-Q16\magick" %1 ( -clone 0 -background #ff00ff ) +swap -background #ff00ff -layers merge +repage -define bmp3:alpha=true BMP3:%1.bmp
