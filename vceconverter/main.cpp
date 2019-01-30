#include <iostream>
using namespace std;
//#include <QCoreApplication>
#include "structss.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>
#define nullptr NULL

char title_text[]="VCE2STR converter v1.2 (C) github.com/ADHSoft .\n";
char readme[]="License GNUGPLv3\n\
The Vce/vci file format is used for UI's and bg animations in the djm. series.\n\
The str format is from the R.O. misc. animations. \n\
This format is still being used in the PSVita(tm) DJM. game for the UI. \n\
\n\
This program aims to convert VCEs into STRs so you can edit them\n\
using the existing STR editors, like the buggy 'RO STR Viewer'.\n\
As that app only supports bmp or tga formats, this program can also convert\n\
your images to bmp, w/ pink transparency calling your preinstalled Imagemagick.\n\
\n\
\n\
USAGE: vceconverter -f -d file1.vce (file2.vce) (...) \n\
Parameters: -f : do not convert image files, and don't change filename references.\n\
-d : just mask or unmask a file.\n\
input files: vce or vci (autodetected)\n\
";

class anikey{   //(aka key frame)
public:
    anikey_t data;
    anikey *next;

    anikey(){
        next=nullptr;
    }
    anikey* getlast () {
        //find last node
        anikey *temp = new anikey;
        temp=this;

        while (temp->next!=nullptr) {
            temp=temp->next;
        }

        return temp;
    }
    anikey(anikey *a){ //create and link node
        anikey *last = new anikey , *temp = new anikey;
        last->next=nullptr;
        temp=a->getlast();
        temp->next=last;

    }

};

class texture {
public:
    texture_t data;
    texture *next;

    texture(){
        next=nullptr;
    }
    texture* getlast () {
        //find last node
        texture *temp = new texture;
        temp=this;

        while (temp->next!=nullptr) {
            temp=temp->next;
        }

        return temp;
    }
    texture(texture *a){ //create and link node
        texture *last = new texture , *temp = new texture;
        last->next=nullptr;
        temp=a->getlast();

        temp->next=last;
    }

};

class layer {
    public:
    char vci_layer_data[0x5c];
    unsigned long texcnt;
    texture firsttex;
    layer *next;
    unsigned long anikeynum=0;
    anikey firstanikey;
    layer(){
        this->next=nullptr;
    }

    layer* getlast () {
        //find last node
        layer *temp = new layer;
        temp=this;

        while (temp->next!=nullptr) {
            temp=temp->next;
        }

        return temp;
    }
    layer(layer *a){ //create and link node
        layer *last = new layer , *temp = new layer;
        last->next=nullptr;
        temp=a->getlast();

        temp->next=last;

    }

};

class vcefile {

public:
    header_t header;
    layer firstlayer;

};

void texname2str(char *name) {
    //append bmp extension
    strcat(name,".bmp");
}

void tex2bmp(char *filename){

    FILE *batfile,*tmp;
    char stringg[200];
    const char bat_prog[]="\
:: This bat file was auto generated by vceconverter \
 because it didn't exist in the folder.\n\
:: Change the Image Magick path if you need.\n\
:: \n:: \n@echo off\n\
\"%%PROGRAMFILES(X86)%%\\ImageMagick-7.0.8-Q16\\magick\" %%1 ( -clone 0 \
-background #ff00ff ) +swap -background #ff00ff -layers merge +repage \
-define bmp3:alpha=false BMP3:%%1.bmp\n";

    //make a bat if there is none
    tmp=fopen(".\\img2bmp.bat","r");
    if (tmp==NULL) {
        batfile = fopen ("img2bmp.bat","w") ;
        fprintf(batfile,bat_prog);
        fflush(batfile);
        fclose(batfile);
        printf("\nA bat file has been created. Edit it to change Image Magick path if necessary.\n");
    } else {
        fclose(tmp);
    }

    //create bmp, executing the .bat
    strcpy(stringg,filename);
    strcat(stringg,".bmp");
    tmp=fopen(stringg,"r");
    if (tmp==NULL) {
        strcpy(stringg,"img2bmp.bat ");
        strcat(stringg,filename);
        system(stringg);
        printf("(converting)");
    } else {
        fclose(tmp);
    }
}

int vce2str(char*,int);
void unmask_vc(char *);
void printvcq();

FILE *infile;
int just_mask=0,bmp_conv=1,masked,vce,vci,vcq;

int main(int argc, char *argv[])
{
    int argcindex=1;

    unsigned char stringg[100];

    if (argc == 1) {	//no parameter, print help
        printf("%s",title_text);
         printf("%s",readme);
         return -1;

    } else { //parse params
        printf("%s",title_text);

        while (argcindex<argc) {
            if (strcmp(argv[argcindex],"-d")==0) {
                just_mask=1;
            } else {
                if (strcmp(argv[argcindex],"-f")==0) {
                    bmp_conv=0;
                } else {
                    infile = fopen (argv[argcindex],"rb");
                    if (infile==nullptr) {
                        printf("Error opening file %s\n",argv[argcindex]);
                        return -1;
                    } else {    //check format
                        printf("Opened %s \n",argv[argcindex]);
                        fread(stringg,sizeof(char),0x40,infile);
                        masked=0;
                        vce=0;
                        vci=0;
                        vcq=0;
                        if (stringg[0]=='V'&&stringg[1]=='C'&&stringg[2]=='M') { //vce/i
                            printf("Type: ");
                            if (stringg[13]!=0) {
                                masked=1;
                            } else {
                                printf("Unmasked ");
                            }
                            char a;
                            a=argv[argcindex][strlen(argv[argcindex])-1];
                            if ( a=='I' || a=='i' ) {
                                vci=1;
                                printf("VCI file\n");
                            } else {
                                vce=1;
                                printf("VCE file\n");
                            }

                            rewind(infile);

                            if (just_mask==0) {
                                if (masked==1) unmask_vc(argv[argcindex]);
                                vce2str(argv[argcindex],vci);
                                if (masked==1) unmask_vc(argv[argcindex]);//re mask
                            } else {
                                unmask_vc(argv[argcindex]);//mask
                            }

                        } else {    //not vce nor vci
                            if (stringg[0]=='V'&&stringg[1]=='C'&&stringg[2]=='Q') {
                                printf("Type: VCQ file\n");
                                printvcq();
                            } else {
                                printf("Unknown format\n");
                            }
                        }
                    }
                    fclose(infile);
                }
            }
            argcindex++;
        }
    }

    return 0;
}

int vce2str(char *path,int vci) {

    vcefile vce;
    char stringg[200];
    FILE *outfile ;

    strcpy(stringg,path);
    strcat(stringg,".str");
    outfile = fopen (stringg,"wb") ;

    if (outfile==nullptr) {
        printf("Error writing file\n");
        return -1;
    }

    fread(&vce.header, sizeof(struct header_s), 1, infile);

    fputs("STRM",outfile);
    fputc(0x94,outfile);
    fputc(0,outfile);
    fputc(0,outfile);
    fputc(0,outfile);
    fwrite(&vce.header.fps,sizeof (unsigned long),1,outfile);
    fwrite(&vce.header.maxkey,sizeof (unsigned long),1,outfile);
    fwrite(&vce.header.layernum,sizeof (unsigned long),1,outfile);
    fwrite(&vce.header.none2,sizeof (unsigned long),3,outfile);
    fwrite(&vce.header.none2,sizeof (unsigned long),1,outfile);

    printf("Total layers: %lu . Max frame time: %lu .\n",vce.header.layernum,vce.header.maxkey);

    int i;

    //read layers
    for (i=0 ; i < vce.header.layernum ; i++) {

        printf("Layer %d ",i);

        if (i!=0) layer(&vce.firstlayer) ; //new layer and link it

        if (vci==1) fread(vce.firstlayer.getlast()->vci_layer_data, sizeof(char), 0x5c, infile);
        if (strcmp(vce.firstlayer.getlast()->vci_layer_data,"#MOVIE#")==0) printf("\"movie\"");

        fread(&(vce.firstlayer.getlast()->texcnt), sizeof(unsigned long), 1, infile);    //texcnt
        fwrite(&(vce.firstlayer.getlast()->texcnt),sizeof (unsigned long),1,outfile);

        printf(": %lu texture(s) : ",vce.firstlayer.getlast()->texcnt);
        //read textures of layer
        int j;

        char strtexname[0x80];
        for (j=0;j<0x80;j++) strtexname[j]=0;

        for (j=0 ; j < vce.firstlayer.getlast()->texcnt ; j++) {
            if (j!=0) texture(&(vce.firstlayer.getlast()->firsttex)) ; //create tex and chain it
            fread(&(vce.firstlayer.getlast()->firsttex.getlast()->data), sizeof(struct texture_s), 1, infile);
            printf("%s ",vce.firstlayer.getlast()->firsttex.getlast()->data.texname);
            strcpy(strtexname,vce.firstlayer.getlast()->firsttex.getlast()->data.texname);
            if (bmp_conv == 1){
                tex2bmp(strtexname);
                texname2str(strtexname);
            }
            fwrite(strtexname, 0x64, 1, outfile);
            //store texcoord at last 1Ch bytes
            fwrite(vce.firstlayer.getlast()->firsttex.getlast()->data.texcoord, 0x1c, 1, outfile);
        }

        //anikeynum
        fread(&(vce.firstlayer.getlast()->anikeynum), sizeof(unsigned long), 1, infile);
        fwrite(&(vce.firstlayer.getlast()->anikeynum),sizeof (unsigned long),1,outfile);
        printf("\n(Key)Frame(s): %lu \n",vce.firstlayer.getlast()->anikeynum);

        //read anikeys of layer
        for (j=0 ; j < vce.firstlayer.getlast()->anikeynum ; j++) {
            if (j!=0) anikey(vce.firstlayer.getlast()->firstanikey.getlast()) ; //create anikey and chain it
            fread(&(vce.firstlayer.getlast()->firstanikey.getlast()->data), sizeof(struct anikey_s), 1, infile);
            fwrite(&(vce.firstlayer.getlast()->firstanikey.getlast()->data), sizeof(struct anikey_s), 1, outfile);
        }

    }


    fflush(outfile);
    fclose(outfile);

}

void unmask_vc(char *path){

    #define MASK_LENGTH 256
    const unsigned char mask[] = {
        247 ,77 ,219 ,74 ,220 ,102 ,240 ,83 ,197 ,127 ,233 ,28 ,138 ,48 ,166 ,5 ,147 ,41 ,191 ,46 ,184 ,2 ,148 ,55 ,161 ,27 ,141 ,0 ,150 ,44 ,186 ,25 ,143 ,53 ,163 ,50 ,164 ,30 ,136 ,43 ,189 ,7 ,145 ,100 ,242 ,72 ,222 ,125 ,235 ,81 ,
        199 ,86 ,192 ,122 ,236 ,79 ,217 ,99 ,245 ,200 ,94 ,228 ,114 ,209 ,71 ,253 ,107 ,250 ,108 ,214 ,64 ,227 ,117 ,207 ,89 ,172 ,58 ,128 ,22 ,181 ,35 ,153 ,15 ,158 ,8 ,178 ,36 ,135 ,17 ,171 ,61 ,144 ,6 ,188 ,42 ,137 ,31 ,165 ,51 ,162 ,
        52 ,142 ,24 ,187 ,45 ,151 ,1 ,244 ,98 ,216 ,78 ,237 ,123 ,193 ,87 ,198 ,80 ,234 ,124 ,223 ,73 ,243 ,101 ,88 ,206 ,116 ,226 ,65 ,215 ,109 ,251 ,106 ,252 ,70 ,208 ,115 ,229 ,95 ,201 ,60 ,170 ,16 ,134 ,37 ,179 ,9 ,159 ,14 ,152 ,34 ,
        180 ,23 ,129 ,59 ,173 ,32 ,182 ,12 ,154 ,57 ,175 ,21 ,131 ,18 ,132 ,62 ,168 ,11 ,157 ,39 ,177 ,68 ,210 ,104 ,254 ,93 ,203 ,113 ,231 ,118 ,224 ,90 ,204 ,111 ,249 ,67 ,213 ,232 ,126 ,196 ,82 ,241 ,103 ,221 ,75 ,218 ,76 ,246 ,96 ,195 ,85 , 239, 121, 140, 26, 160, 54, 149,
        3 ,185 , 47 ,190 ,40 ,146 ,4 ,167 ,49 ,139 ,29 ,176 ,38 ,156 ,10 ,169 ,63 ,133 ,19 ,130 ,20 ,174 ,56 ,155 ,13 ,183 ,33 ,212 ,66 ,248 ,110 ,205 ,91 ,225 ,119 ,230 ,112 ,202 ,92 ,255 ,105 ,211 ,69 ,120 ,238 ,84 ,194 ,97
    };

    int c, i, k_pos,header;
    char str[200];
    char *buf=NULL;
    long int size,f_pos=0;

	FILE *f;

	f=infile;
	rewind(f);

    //copy file to buf
    fseek(f, 0, SEEK_END);
    size=ftell(f);
    buf=new char[size];
    //malloc(sizeof(char)*size);
    rewind(f);
    while (1) {
        i = fgetc(f);
        if (i==EOF) break;
        buf[f_pos]=i;
        f_pos++;
    }
    fclose(f);
    infile = fopen (path,"wb"); //reopen file

    header= 8 ;	//preserve the header "VC*F\0\x01\x02\0"
    k_pos=0;
    f_pos=0;
    while (1) {	//for every byte
        i=buf[f_pos];
        if (header !=0) {	//preserve header bytes
            header--;
            c = i;
        } else {
                c = i ^ mask[k_pos];	//xor the byte with the mask
        }

        fputc(c,f); //write byte to file

        k_pos++;
        if (k_pos==MASK_LENGTH) {	//loop the mask index
            k_pos=0;
        }

        f_pos++;
        if (f_pos==size) break; //if end of file
    }

    delete buf;
    fflush(f);
    fclose(f);
    infile = fopen (path,"rb"); //reopen file

}

void printvcq(){



}
