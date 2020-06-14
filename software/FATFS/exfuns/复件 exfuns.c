#include "exfuns.h"
#include "fattester.h"	
 

///////////////////////////////�����ļ���////////////////////////////////////////////
FATFS fs[2];  		//�߼����̹�����.	 
FIL file;	  		//�ļ�1
FIL ftemp;	  		//�ļ�2.
UINT br,bw;			//��д����
FILINFO fileinfo;	//�ļ���Ϣ
DIR dir;  			//Ŀ¼

u8 fatbuf[512];//SD�����ݻ�����
///////////////////////////////////////////////////////////////////////////////////////


FIL *fc_src=&file;	//Դ�ļ�
FIL *fc_dst=&ftemp;	//Ŀ���ļ�
u8 *fc_buf=fatbuf;	//���ݻ�����  

//�ļ�����·��������
struct _m_pfnmg pfnmg=
{
	pfnmg_init,
	pfnmg_dir_opendir,
	pfnmg_get_pname,
	pfnmg_get_plfname,
	pfnmg_get_lfname,
	pfnmg_get_ifname,
	pfnmg_scan_folder,
	0,
	0,
	0,
	0,
	0,
	0,	
};

//��ʼ��·��
//������'/'��β!
//path:·��;
//����ֵ:0,OK;����,ʧ��.
u8 pfnmg_init(u8*path)
{
	u8 res;
	res=f_opendir(&dir,(const TCHAR*)path);
	if(res)return res;//��·���Ƿ�
	else
	{										  
		pfnmg.cftp=0XFF; 	//��ǰΪ·��
		pfnmg.foldernum=0; 	//���ļ�������
		pfnmg.dfnum=0;	   	//Ŀ���ļ�������
		pfnmg.fttblsize=0; 	//Ŀ���ļ����ͱ����
		f_strcpy(pfnmg.plfname,path);//·�����Ƹ�pname	
	}
	return 0;
}
//�򿪵�ǰĿ¼�µ�ĳ���ļ���.
//dirname:�ļ�������
//����ֵ:0,�ɹ�;����,ʧ��;
u8 pfnmg_dir_opendir(u8*dirname)
{
	u8 res;
	u8 len;
	u8 deep;						   					    
	res=f_opendir(&dir,(const TCHAR*)dirname);//�򿪴��ļ���
	if(res==0)//·��OK
	{
		len=f_strlen(dirname); 
		if(dirname[len-1]=='.'&&dirname[len-2]=='.')//�ص���һ��Ŀ¼.
		{	    
			deep=f_getdirdeep(dirname);//�õ�Ŀ¼���
			if(deep)//�����Ǹ�Ŀ¼
			{
				if(deep>1)deep--;						//�õ��������Ŀ¼
				f_getdirstr(dirname,dirname,deep-1); //�õ���һ��Ŀ¼	
			}else dirname[len-2]=0;//��ӽ�����.	 
		}
		f_strcpy(pfnmg.plfname,dirname);//����·��
 		pfnmg.cftp=0XFF; 	//��ǰΪ·��
		pfnmg.foldernum=0; 	//���ļ�������
		pfnmg.dfnum=0;	   	//Ŀ���ļ�������
		pfnmg_scan_folder();//ɨ��������ļ���.
	}
	return res;		
}

//�õ���ǰ·����
u8* pfnmg_get_pname(void) 
{
	u8 len;
	if(pfnmg.cftp!=0XFF)//��ǰ����ȫ��·����
	{
		len=f_strlen(pfnmg.plfname);
		while(pfnmg.plfname[len]!='/')len--;
		pfnmg.plfname[len]='\0';//�ӽ�����.
		pfnmg.cftp=0XFF;//��ǰ��·��
	} 
	return pfnmg.plfname;
}   
//�õ�ĳ���ļ����Ĵ�·���������
//����ֵ:·���ļ����׵�ַ.
u8* pfnmg_get_plfname(void)
{										  		 
 	return pfnmg.plfname;		    
}
//�õ��ļ���
//����ֵ:�ļ����׵�ַ.
u8* pfnmg_get_lfname(void)
{
	u8 len;
	if(pfnmg.cftp==0XFF)return 0;//��ǰ��ȫ��·����
	else
	{
		len=f_strlen(pfnmg.plfname);
		while(pfnmg.plfname[len]!='/')len--; 
		return pfnmg.plfname+len+1;//�����ļ���  
	} 							    
}   
//�õ�ָ������ļ����ļ��� ���:0~dfnum+foldernum-1
//index:�ļ����(�������ļ���)
//*ftpt:���ͱ�.
//num:������
//����ֵ:���ļ����ļ���.
u8* pfnmg_get_ifname(u16 index)
{
	FILINFO finfo;
	u16 i=0;
	u8 t,ftype;
	u8 res;
	u8 tlfname[100];//100���ֽ����ڴ�ų��ļ���
	u8 *plfname;

	plfname=pfnmg_get_pname();//�õ���ǰ·����    
	res=f_opendir(&dir,(const TCHAR*)plfname);//�򿪵�ǰĿ¼
    finfo.lfsize=0;//��ʹ�ó��ļ��� 
	if(res==FR_OK)
	{
		if(index<pfnmg.foldernum)//���ļ�����
		{
			for(i=0;i<index+1;)
			{	 
				if(i==index)//�Ѿ����ڵ�ǰҪ�ҵ��ļ���.ʹ�ó��ļ���.
				{
					finfo.lfname=(TCHAR*)tlfname;		//���ļ�������
					finfo.lfsize=sizeof(tlfname);		//�����С
				}
				res=f_readdir(&dir,&finfo);	//��ȡһ���ļ�����Ϣ
		    	if(res!=FR_OK||finfo.fname[0]==0)break;//����,�����Ѿ�������
				if(finfo.fattrib&AM_DIR)
				{
					if(finfo.fname[0]=='.'&&finfo.fname[1]=='\0')continue;//������"."�ļ���
					i++;
				}										    
			}   
		}else //����Ч�ļ���
		{
			index-=pfnmg.foldernum;//ʵ���ļ���ƫ��
			for(i=0;i<index+1;)
			{	 
				if(i==index)//�Ѿ����ڵ�ǰҪ�ҵ��ļ���.ʹ�ó��ļ���.
				{
					finfo.lfname=(TCHAR*)tlfname;		//���ļ�������
					finfo.lfsize=sizeof(tlfname);		//�����С
				}
				res=f_readdir(&dir,&finfo);	//��ȡһ���ļ�����Ϣ
		    	if(res!=FR_OK||finfo.fname[0]==0)break;//����,�����Ѿ�������
				ftype=f_typetell((u8*)finfo.fname);//�õ��ļ�����
				for(t=0;t<pfnmg.fttblsize;t++)if(pfnmg.fttbl[t]==ftype)break;
				if(t!=pfnmg.fttblsize)i++;//�ҵ�һ������������.
 				   									    
			}
		}
		if(res==FR_OK&&finfo.fname[0]!=0)//�ɹ���ȡ��.
		{				    
			f_stradd(pfnmg.plfname,"/");
			if(*finfo.lfname)f_stradd(pfnmg.plfname,(u8*)finfo.lfname);	//���ڳ��ļ���
			else f_stradd(pfnmg.plfname,(u8*)finfo.fname);			   	//�������ļ���
			if(finfo.fattrib&AM_DIR)pfnmg.cftp=T_FOLDER;		  		//�Ǹ��ļ���.	 
			else pfnmg.cftp=f_typetell((u8*)finfo.fname);		  		//Ŀ���ļ�
		}  
	}
 	return pfnmg_get_lfname();//���صõ����ļ���.	
}   
//ɨ���ļ���
//ftpt:�ļ���������
//num:�ļ�������
//����ֵ:�ļ�����(�����ļ���)
u16 pfnmg_scan_folder(void)
{
	u16 fcnt;  
	u8 i;
	u8 *pname; 

	pname=pfnmg_get_pname();		//�õ�·����
	fcnt=f_getfoldernum(pname);		//�õ���ǰ�ļ�����,�ļ��еĸ��� 
	pfnmg.foldernum=fcnt;			//Ŀ¼����
	for(i=0;i<pfnmg.fttblsize;i++)	//�õ�Ŀ���ļ��ĸ���
	{
		fcnt+=f_getfilenum(pname,pfnmg.fttbl[i]); 
	} 
 	pfnmg.dfnum=fcnt-pfnmg.foldernum;//Ŀ���ļ��ĸ���
	return fcnt;	
}    		  

//�����ַ���.
//��str2������,���Ƶ�str1����.
//����ֵ:0,�ɹ�;����ʧ��.
void f_strcpy(u8*str1,u8*str2)
{				  
	while(*str2!='\0')
	{
		*str1=*str2;
		str1++;		 
		str2++;
	}
	*str1='\0';//ĩβ���������.  
}	
//�õ�ĳ���ַ����ĳ���
//str:�ַ���;
//����ֵ:����.
u8 f_strlen(u8*str)
{
	u8 strlen=0;
	while(*str!='\0')
	{
		strlen++;
		str++;
	}
	return strlen;
}					
//�Ա������ַ���,�Ƿ����.
//���,����1;����,����0.
u8 f_strcmp(u8*str1,u8 *str2)
{
	while(1)
	{
		if(*str1!=*str2)return 0;//�����
		if(*str1=='\0')break;//�Ա������.
		str1++;
		str2++;
	}
	return 1;//�����ַ������
}
//��Сд��ĸתΪ��д��ĸ,���������,�򱣳ֲ���.
u8 f_upper(u8 c)
{
	if(c<'A')return c;//����,���ֲ���.
	if(c>='a')return c-0x20;//��Ϊ��д.
	else return c;//��д,���ֲ���
}
//���ַ���str2��ӵ�str1�ĺ���.	 
u8 f_stradd(u8 *str1,u8 *str2) 
{
	while(*str1!='\0')str1++;
	while(*str2!='\0')
	{
		*str1=*str2;
		str1++;
		str2++;
	}
	*str1='\0';	//ĩβ��ӽ�����
	return 0;	//��ӳɹ�.
}
//����ĳ��Ŀ¼���Ƿ���ĳ���ļ�
//����ֵ0,��.����,û��.
u8 f_find(u8*fpath)
{
	u8 res;
	res=f_open(fc_src,(const TCHAR*)fpath,FA_READ);
	f_close(fc_src);
	return res;		    
}
//�����ļ�
//fsrc:Դ�ļ�·��
//fdst:Ŀ���ļ�·��
//func:״̬��ʾ����ָ��. u32(*prog_func)(u32,u32);,��һ������Ϊ�ļ���С,�ڶ�������Ϊ��ǰ���е�λ��.
//����ֵ:0,OK,����,ʧ��.  
u8 f_copy(u8 *fsrc,u8* fdst,prog_funs prog_fun)  
{
	u8 res=0;
	u32 offset=0;
 	res=f_open(fc_src,(const TCHAR*)fsrc,FA_READ);//ֻ����ʽ��
	if(res==FR_OK)//�ɹ������ļ�,�ſ�ʼ.
	{   
	 	res=f_open(fc_dst,(const TCHAR*)fdst,FA_WRITE|FA_CREATE_ALWAYS);//�½�һ���ļ�
 		while(res==FR_OK)//��ѭ��ִ��
		{
	 		res=f_read(fc_src,fc_buf,512,&br);	 
			if(res!=FR_OK)break;				//ִ�д���	  
	 		res=f_write(fc_dst,fc_buf,br,&bw);	//д�� 
			if(res!=FR_OK)break;				//ִ�д���
			if(br!=bw){res=0xff;break;}			//д�����
			offset+=br;	  
			prog_fun(fc_src->fsize,offset);		//������ʾ
			if(br!=512)break;					//������.
			f_lseek(fc_src,offset);//ƫ�Ƶ���һ�ζ��ĵ�ַ
		}  
		f_close(fc_dst);
	}
	f_close(fc_src);
	return res;
}
//�õ�Ŀ¼�����
//pathname:·����
//����ֵ:0,��һ��Ŀ¼(��Ŀ¼);1,�ڶ���Ŀ¼...254,��254��Ŀ¼
u8 f_getdirdeep(u8*pathname)
{
	u8 dpf=0;
	while(1)
	{
		if(*pathname=='\0')break;//��ĩβ��
		if(*pathname=='/')dpf++; //�ҵ�һ�����ַ�
		pathname++; 		
	}	
	return dpf;
}	   

//�õ��ڼ�����Ŀ¼.
//dsrc:Դ�ַ���.(·��)
//ddst:�ڼ���Ŀ¼�ַ���(·��)
//deep:0,��һ��Ŀ¼;1,�ڶ���Ŀ¼...254,��254��Ŀ¼
//����ֵ:��deep���,��ɹ�,0XFF,ʧ��.
u8 f_getdirstr(u8* dsrc,u8 *ddst,u8 deep) 
{
	u8 dpf=0;
	while(1)
	{
		if(*dsrc=='\0')
		{
			if(dpf)dpf--;		//���ҵ��˽�����,����Ŀ¼�ָ���־.
			break;				//��ĩβ��	   
		}
		if(*dsrc=='/')			//�ҵ�һ�����ַ�
		{
			if(dpf==deep)break;	//Ŀ¼��������,�˳�
			dpf++; 				//Ŀ¼�������
		}
		*ddst=*dsrc;			//copy			   
		ddst++;
		dsrc++;
	}
	if(dpf!=deep)return 0XFF;
	else 
	{
		*ddst='\0';//���������.
		return dpf;
	}
}	
		 
const u8 *FILE_TYPE_TAB[4][13]=
{
{"MP1","MP2","MP3","MP4","M4A","3GP","3G2","OGG","ACC","WMA","WAV","MID","FLAC"},
{"LRC","TXT","C","H"},
{"BIN","FON","MP3"},
{"BMP","JPG","JPEG"},
};	    
//�����ļ�������
//fname:�ļ���
//����ֵ:�ļ����ͱ��.0XFF,��ʾ�޷�ʶ��.
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 i=0,j;
	while(i<200)
	{
		i++;
		if(*fname=='\0')break;//ƫ�Ƶ��������.
		fname++;
	}
	if(i==200)return 0XFF;//������ַ���.
	fname--;		//���һ���ַ���λ��
	for(i=0;i<5;i++)//���ƺ�׺��
	{
		tbuf[4-i]=*fname;
		fname--;
	}
	if(tbuf[0]=='.')//��׺��Ϊ4���ֽ�
	{
		tbuf[0]=f_upper(tbuf[1]);
		tbuf[1]=f_upper(tbuf[2]);
		tbuf[2]=f_upper(tbuf[3]);
		tbuf[3]=f_upper(tbuf[4]);
		tbuf[4]='\0';//���������
	}else if(tbuf[1]=='.')//3���ֽ�
	{
		tbuf[0]=f_upper(tbuf[2]);
		tbuf[1]=f_upper(tbuf[3]);
		tbuf[2]=f_upper(tbuf[4]);  
		tbuf[3]='\0';//���������
	}else if(tbuf[2]=='.')//2���ֽ�
	{
		tbuf[0]=f_upper(tbuf[3]);
		tbuf[1]=f_upper(tbuf[4]);   
		tbuf[2]='\0';//���������
	}else if(tbuf[3]=='.')//1���ֽ�
	{
		tbuf[0]=f_upper(tbuf[4]);    
		tbuf[1]='\0';//���������
	}else return 0XFF;//δ�ҵ���׺��.
	for(i=0;i<4;i++)
	{
		for(j=0;j<13;j++)
		{
			if(*FILE_TYPE_TAB[i][j]==0)break;//�����Ѿ�û�пɶԱȵĳ�Ա��.
			if(f_strcmp((u8 *)FILE_TYPE_TAB[i][j],tbuf))//�ҵ���
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//û�ҵ�
		 			   
}
//�õ�ָ��Ŀ¼�µ�ָ�������ļ��ĸ���
u16 f_getfilenum(u8* path,u8 ftype)
{
	u8 res;
	u8 type;
	u16 fnum=0;
	FILINFO finfo;
	res=f_opendir(&dir,(const TCHAR*)path);//��SD���ϵ�PICTURE�ļ���.
    finfo.lfsize=0;//��ʹ�ó��ļ��� 
	if(res==FR_OK)
	{
		while(1)
		{	 
			res=f_readdir(&dir,&finfo);	//��ȡһ���ļ�����Ϣ
	    	if(res!=FR_OK||finfo.fname[0]==0)break;//����,�����Ѿ�������
			type=f_typetell((u8*)finfo.fname);//�õ��ļ�����	 
			if(type==ftype)fnum++;//�������ļ���������1.
		}
	}
    return fnum;
}			
//�õ�ָ��Ŀ¼�µ��ļ��еĸ���
//����".."�ļ���
u16 f_getfoldernum(u8* path)
{
	u8 res;	    
	u16 fnum=0;
	FILINFO finfo;
	res=f_opendir(&dir,(const TCHAR*)path);//��SD���ϵ�PICTURE�ļ���.
    finfo.lfsize=0;//��ʹ�ó��ļ��� 
	if(res==FR_OK)
	{
		while(1)
		{

			res=f_readdir(&dir,&finfo);	//��ȡһ���ļ�����Ϣ
	    	if(res!=FR_OK||finfo.fname[0]==0)break;//����,�����Ѿ�������
			if(finfo.fattrib&AM_DIR)
			{
				if(finfo.fname[0]=='.'&&finfo.fname[1]=='\0')continue;//������"."�ļ���
				fnum++;
			}										    
		}
	}
    return fnum;
}									    








