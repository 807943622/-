#include "fattester.h"	 
#include "mmc_sd.h"
#include "usart.h"
#include "exfuns.h"

#include "ff.h"


//���ش���	   
u8 mf_mount(u8 drv)
{		   
	return f_mount(drv,&fs[drv]); 
}
//��·���µ��ļ�
u8 mf_open(u8*path,u8 mode)
{
	u8 res;
	res=f_open(&file,(const TCHAR*)path,mode);//���ļ���
	return res;
} 
//�ر��ļ�
u8 mf_close(void)
{
	f_close(&file);
	return 0;
}
u8 mf_read(u16 len)
{
	u16 i,t;
	u8 res;
	u16 tlen=0;
	printf("\r\nRead file data is:\r\n");
	for(i=0;i<len/512;i++)
	{
		res=f_read(&file,fatbuf,512,&br);
		if(res)
		{
			printf("Read Error:%d\r\n",res);
			break;
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}
	}
	if(len%512)
	{
		res=f_read(&file,fatbuf,len%512,&br);
		if(res)	//�����ݳ�����
		{
			printf("Read Error:%d\r\n",res);   
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}	 
	}
	if(tlen)printf("Readed data len:%d\r\n",tlen);//���������ݳ���
	printf("Read data over\r\n");	 
	return res;
}
//д������
u8 mf_write(u8*dat,u16 len)
{			    
	u8 res;	   					   

	printf("\r\nWriting file data.\r\n");
	printf("Write data len:%d\r\n",len);	 
	res=f_write(&file,dat,len,&bw);
	if(res)
	{
		printf("Write Error:%d\r\n",res);   
	}else printf("Writed data len:%d\r\n",bw);
	printf("Write data over.\r\n");
	return res;
}

//���ļ���
u8 mf_opendir(u8* path)
{
	return f_opendir(&dir,(const TCHAR*)path);	
}
//���ȡ�ļ���
u8 mf_readdir(void)
{
	u8 res;
	char *fn;			 
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  
	res=f_readdir(&dir,&fileinfo);//��ȡһ���ļ�����Ϣ
	if(res!=FR_OK||fileinfo.fname[0]==0)
	{
		myfree(SRAMIN,fileinfo.lfname);
		return res;//������.
	}
#if _USE_LFN
	fn=*fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
	fn=fileinfo.fname;;
#endif	
	printf("\r\n DIR info:\r\n");

	printf("dir.id:%d\r\n",dir.id);
	printf("dir.index:%d\r\n",dir.index);
	printf("dir.sclust:%d\r\n",dir.sclust);
	printf("dir.clust:%d\r\n",dir.clust);
	printf("dir.sect:%d\r\n",dir.sect);	  

	printf("\r\n");
	printf("File Name is:%s\r\n",fn);
	printf("File Size is:%d\r\n",fileinfo.fsize);
	printf("File data is:%d\r\n",fileinfo.fdate);
	printf("File time is:%d\r\n",fileinfo.ftime);
	printf("File Attr is:%d\r\n",fileinfo.fattrib);
	printf("\r\n");
	myfree(SRAMIN,fileinfo.lfname);
	return 0;
}			 

 //�����ļ�
u8 mf_scan_files(u8 * path)
{
	FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  

    res = f_opendir(&dir,(const TCHAR*)path); //��һ��Ŀ¼
    if (res == FR_OK) 
	{	
		printf("\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
	        //if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			printf("%s/", path);//��ӡ·��	
			printf("%s\r\n",  fn);//��ӡ�ļ���	
		} 
    }	  
	myfree(SRAMIN,fileinfo.lfname);
    return res;	  
}
//��ʾʣ������
u32 mf_showfree(u8 *drv)
{
	FATFS *fs1;
	u8 res;
    u32 fre_clust, fre_sect, tot_sect;
    //�õ�������Ϣ�����д�����
    res = f_getfree((const TCHAR*)drv, &fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;//�õ���������
	    fre_sect = fre_clust * fs1->csize;			//�õ�����������	   
#if _MAX_SS!=512
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		if(tot_sect<20480)//������С��10M
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n����������:%d KB\r\n"
		           "���ÿռ�:%d KB\r\n",
		           tot_sect>>1,fre_sect>>1);
		}else
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n����������:%d MB\r\n"
		           "���ÿռ�:%d MB\r\n",
		           tot_sect>>11,fre_sect>>11);
		}
	}
	return fre_sect;
}
//�ļ���дָ��ƫ��
//offset:�׵�ַƫ�Ƶ���
//����ֵ:ִ�н��.
u8 mf_lseek(u32 offset)
{
	return f_lseek(&file,offset);
}
//��ȡ�ļ���ǰ��дָ���λ��.
//����ֵ:λ��
u8 mf_tell(void)
{
	return f_tell(&file);
}
//��ȡ�ļ���С
//����ֵ:�ļ���С
u32 mf_size(void)
{
	return f_size(&file);
} 
//����Ŀ¼
u8 mf_mkdir(u8*name)
{
	return f_mkdir((const TCHAR *)name);
}
//ɾ���ļ�/Ŀ¼
u8 mf_unlink(u8 *name)
{
	return  f_unlink((const TCHAR *)name);
}
//�޸��ļ�/Ŀ¼����(���Ŀ¼��ͬ,�������ƶ��ļ�Ŷ!)
//oldname:֮ǰ������
//newname:������
u8 mf_rename(u8 *oldname,u8* newname)
{
	return  f_rename((const TCHAR *)oldname,(const TCHAR *)newname);
}

//���ļ������ȡһ���ַ���
void mf_gets(u16 size)
{
	u16 i;
	TCHAR* rbuf;
	rbuf=f_gets((TCHAR*)fatbuf,size,&file);
	if(*rbuf==0)return  ;//û�����ݶ���
	else
	{
		printf("\r\nGet String Is:\r\n");
		for(i=0;i<size;i++)
		{
			printf("%02x ",fatbuf[i]);
		}
		printf("\r\n");
	}			    	
}
//��Ҫ_USE_STRFUNC>=1
//дһ���ַ����ļ�
u8 mf_putc(u8 c)
{
	return f_putc((TCHAR)c,&file);
}
//д�ַ������ļ�
u8 mf_puts(u8*c)
{
	return f_puts((TCHAR*)c,&file);
}
//��nsrc�ļ�,copy��ndst.
u8 mf_copy(u8 *nsrc,u8 *ndst)
{
	u8 res;
	res=f_open(&file,(const TCHAR*)nsrc,FA_READ|FA_OPEN_EXISTING);
 	res=f_open(&ftemp,(const TCHAR*)ndst,FA_WRITE|FA_CREATE_ALWAYS);
 	while(res==0)
	{
		res=f_read(&file,fatbuf,sizeof(fatbuf),&br);//Դͷ����512�ֽ�
		if(res||br==0)break;
		res=f_write(&ftemp,fatbuf,br,&bw);//д��Ŀ���ļ�
		if(res||bw<br)break;       
	}
    f_close(&file);
    f_close(&ftemp);
	return res;
} 












