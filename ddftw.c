/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "ddftw.h"
#include "common.h"
#include "extra-functions.h"
#include "bootloader.h"

// get locations from our device.info
void getLocations()
{
	FILE *fp;
	int tmpInt;
	char tmpText[50];
	if (strcmp(get_fstype(),"mtd") == 0)
	{
		fp = __popen("cat /proc/mtd", "r");
	} else if (strcmp(get_fstype(),"emmc") == 0) {
		fp = __popen("cat /proc/emmc", "r");
	}
	if (fp == NULL)
	{
		ui_print("\n=> Halp! Could not determine flash type!\n");
	} else {
		while (fscanf(fp,"%s %*s %*s %*c%s",tmp.dev,tmp.mnt) != EOF)
		{
			if (strcmp(tmp.dev,"dev:") != 0)
			{
				tmp.dev[strlen(tmp.dev)-1] = '\0';
				tmp.mnt[strlen(tmp.mnt)-1] = '\0';
				if (sscanf(tmp.dev,"mtd%d",&tmpInt) == 1)
				{
					sprintf(tmpText,"%smtdblock%d",tw_block,tmpInt);
					strcpy(tmp.blk,tmpText);
					sprintf(tmpText,"%s%s",tw_mtd,tmp.dev);
					strcpy(tmp.dev,tmpText);
				} else {
					sprintf(tmpText,"%s%s",tw_block,tmp.dev);
					strcpy(tmp.dev,tmpText);
					strcpy(tmp.blk,tmp.dev);
				}
			}
			if (strcmp(tmp.mnt,"system") == 0) { // read in system line
				strcpy(sys.mnt,tmp.mnt);
				strcpy(sys.dev,tmp.dev);
				strcpy(sys.blk,tmp.blk);
			}
			if (strcmp(tmp.mnt,"userdata") == 0) {
				strcpy(dat.mnt,"data");
				strcpy(dat.dev,tmp.dev);
				strcpy(dat.blk,tmp.blk);
			}
			if (strcmp(tmp.mnt,"boot") == 0) {
				strcpy(boo.mnt,tmp.mnt);
				strcpy(boo.dev,tmp.dev);
				strcpy(boo.blk,tmp.blk);
			}
			if (strcmp(tmp.mnt,"recovery") == 0) {
				strcpy(rec.mnt,tmp.mnt);
				strcpy(rec.dev,tmp.dev);
				strcpy(rec.blk,tmp.blk);
			}
			if (strcmp(tmp.mnt,"cache") == 0) {
				strcpy(cac.mnt,tmp.mnt);
				strcpy(cac.dev,tmp.dev);
				strcpy(cac.blk,tmp.blk);
			}
			if (strcmp(tmp.mnt,"wimax") == 0) {
				strcpy(wim.mnt,tmp.mnt);
				strcpy(wim.dev,tmp.dev);
				strcpy(wim.blk,tmp.blk);
			}
		}
		pclose(fp);
		readRecFstab();
	}
	get_device_id();
}

void readRecFstab()
{
	FILE *fp;
	char tmpText[255];
	__system("touch /etc/mtab");
	fp = fopen("/etc/recovery.fstab", "r");
	if (fp == NULL) {
		LOGI("=> Can not open /etc/recovery.fstab.\n");
	} else {
		fgets(tmpText, 255, fp);
		fgets(tmpText, 255, fp);
		while (fgets(tmpText,255,fp) != NULL)
		{
			sscanf(tmpText,"%*c%s %s %s %s",tmp.mnt,tmp.fst,tmp.blk,tmp.dev);
			if (strcmp(tmp.mnt,"system") == 0)
			{
				strcpy(sys.fst,tmp.fst);
				if (strcmp(sys.mnt,"system") != 0)
				{
					strcpy(sys.mnt,"system");
					strcpy(sys.blk,tmp.blk);
					strcpy(sys.dev,tmp.blk);
				}
			}
			if (strcmp(tmp.mnt,"data") == 0)
			{
				strcpy(dat.fst,tmp.fst);
				if (strcmp(dat.mnt,"data") != 0)
				{
					strcpy(dat.mnt,"data");
					strcpy(dat.blk,tmp.blk);
					strcpy(dat.dev,tmp.blk);
				}
			}
			if (strcmp(tmp.mnt,"cache") == 0)
			{
				strcpy(cac.fst,tmp.fst);
				if (strcmp(cac.mnt,"cache") != 0)
				{
					strcpy(cac.mnt,"cache");
					strcpy(cac.blk,tmp.blk);
					strcpy(cac.dev,tmp.blk);
				}
			}
			if (strcmp(tmp.mnt,"sdcard") == 0)
			{
				strcpy(sdc.fst,tmp.fst);
				if (strcmp(sdc.mnt,"sdcard") != 0)
				{
					strcpy(sdc.mnt,"sdcard");
					strcpy(sdc.blk,tmp.blk);
					strcpy(sdc.dev,tmp.dev);
				}
			}
		}
	}
	fclose(fp);
	strcpy(ase.dev,"/sdcard/.android_secure");
	strcpy(ase.mnt,".android_secure");
	strcpy(sde.mnt,"sd-ext");
	int tmpInt;
	char tmpBase[50];
	char tmpWildCard[50];
	strcpy(tmpBase,sdc.blk);
	tmpBase[strlen(tmpBase)-1] = '\0';
	sprintf(tmpWildCard,"%s%%d",tmpBase);
	sscanf(sdc.blk,tmpWildCard,&tmpInt);
	sprintf(sde.blk,"%s%d",tmpBase,tmpInt+1);
	createFstab();
}

// write fstab so we can mount in adb shell
void createFstab()
{
	FILE *fp;
	struct stat st;
	fp = fopen("/etc/fstab", "w");
	if (fp == NULL) {
		LOGI("=> Can not open /etc/fstab.\n");
	} else {
		char tmpString[255];
		sprintf(tmpString,"%s /%s %s rw\n",sys.blk,sys.mnt,sys.fst);
		fputs(tmpString, fp);
		sprintf(tmpString,"%s /%s %s rw\n",dat.blk,dat.mnt,dat.fst);
		fputs(tmpString, fp);
		sprintf(tmpString,"%s /%s %s rw\n",cac.blk,cac.mnt,cac.fst);
		fputs(tmpString, fp);
		sprintf(tmpString,"%s /%s %s rw\n",sdc.blk,sdc.mnt,sdc.fst);
		fputs(tmpString, fp);
		if (stat(sde.blk,&st) == 0)
		{
			sprintf(tmpString,"%s /%s %s rw\n",sde.blk,sde.mnt,sde.fst);
			fputs(tmpString, fp);
		}
	}
	fclose(fp);
	LOGI("=> /etc/fstab created.\n");
}
