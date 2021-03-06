//  Copyright (C) 2010 Lothar Braun <lothar@lobraun.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "dump_classes.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <tools/pcap-tools.h>
#include <tools/msg.h>

#define MAX_FILENAME 65535

uint64_t convert_to_uint64_t(const char* value)
{
	// a value can be ended with K, M, G with stands for Kilo, Mega or Giga
	// it is therefore possible to encode 10G without writing all the zeros.
	uint64_t ret = 0;
	size_t len = strlen(value);
	char* copy = (char*)malloc(len);
	strncpy(copy, value, len);

	if (len == 0) {
		goto out1;
	}
	char trailing = copy[len - 1];
	if (trailing == 'K' || trailing == 'G' || trailing == 'M') {
		copy[len - 1] = 0;
	} 
	ret = atoll(copy);
	if (trailing == 'K') {
		ret *= 1000;
	} else if (trailing == 'M') {
		ret *= 1000*1000; 
	} else if (trailing == 'G') {
		ret *= (uint64_t)1000*(uint64_t)1000*(uint64_t)1000;	
	}

	free(copy);
	return ret;
out1:
	free(copy);
	return 0;
}

list_t* classes_create(const char* module_name, struct config* c, int linktype)
{
        list_t* ret;
        uint32_t class_no;
        uint32_t class_count;
        const char* class_name;
        const char* filter_string;
        const char* prefix;
	const char* tmp;
	uint32_t cutoff;
	char conf_name[MAX_FILENAME];	
	uint64_t file_size = 0;
	uint64_t disk_size = 0;
	uint8_t is_stdout = 0;
	pcap_t* p;

        ret = list_create();
        if (!ret) {
                msg(MSG_ERROR, "%s: Could not create list: %s", module_name, strerror(errno));
                goto out1;
        }

        if (!config_get_option(c, module_name, "number_of_classes")) {
                msg(MSG_ERROR, "%s: missing \"number_of_classes\". Cannot configure %s", module_name, module_name);
                goto out2;
        } else {
                class_count = atoi(config_get_option(c, module_name, "number_of_classes"));
        }

        prefix = config_get_option(c, module_name, "file_prefix");
        if (!prefix) {
                msg(MSG_ERROR, "%s: missing \"file_prefix\". Cannot configure %s", module_name, module_name);
                goto out2;
        }
	if (strlen(prefix) == 1 && !strcmp(prefix, "-")) {
		is_stdout = 1;
		msg(MSG_DEBUG, "%s outputing to stdout!", module_name);
	}

        p = pcap_open_dead(linktype, 65535);
        // build filters from module confiugration
        // open pcap files for every defined class
        for (class_no = 1; class_no <= class_count; ++class_no) {
                snprintf(conf_name, MAX_FILENAME, "class%d", class_no);
                class_name = config_get_option(c, module_name, conf_name);
                if (!class_name) {
                        msg(MSG_ERROR, "%s: could not find %s in config file!", module_name, conf_name);
                        goto out2;
                }

                snprintf(conf_name, MAX_FILENAME, "filter%d", class_no);
                filter_string = config_get_option(c, module_name, conf_name);
                if (!filter_string) {
                        msg(MSG_ERROR, "%s: Could not find filter expression for class %s. Cannot recover from that!", module_name, class_name);
                        goto out2;
                }

		snprintf(conf_name, MAX_FILENAME,  "cutoff%d", class_no);
		tmp = config_get_option(c, module_name, conf_name);
		if (!tmp) {
			cutoff = 0;
		} else {
			cutoff = atoi(tmp);
			msg(MSG_INFO, "Cutoff for class %s is %d", class_name, cutoff);
		}
		
		snprintf(conf_name, MAX_FILENAME, "file_size%d", class_no);
		tmp = config_get_option(c, module_name, conf_name);
		if (tmp) {
			file_size = convert_to_uint64_t(tmp);
			msg(MSG_INFO, "File size for class %s is %llu", class_name, file_size);
		}

		snprintf(conf_name, MAX_FILENAME, "disk_size%d", class_no);
		tmp = config_get_option(c, module_name, conf_name);
		if (tmp) {
			disk_size = convert_to_uint64_t(tmp);
			msg(MSG_INFO, "Disk size for class %s is %llu", class_name, disk_size);
		}

		if (disk_size < file_size) {
			msg(MSG_ERROR, "Filesize is greater than disk size. This is an invalid value");
			goto out2;
		}
		
                struct list_element_t* le = (struct list_element_t*)malloc(sizeof(struct list_element_t));
                struct class_t* f = (struct class_t*)malloc(sizeof(struct class_t));
                if (-1 == pcap_compile(p, &f->filter_program, filter_string,  0, 0)) { // TODO: check whether optimize in pcap_compile could be usefull
                        msg(MSG_ERROR, "%s: Could not compile pcap filter %s: %s", module_name, filter_string, pcap_geterr(p));
                        // TODO: cleanup this one, too 
                        goto out2;
                }

               	f->prefix = prefix;
		f->is_stdout = is_stdout;
		f->class_name = class_name;
		f->cutoff = cutoff;
		f->file_size = file_size;
		f->disk_size = disk_size;
		f->suffix = 0;
		f->file_traffic_seen = 0;
		f->disk_traffic_seen = 0;
		f->linktype = linktype;
		snprintf(conf_name, MAX_FILENAME, "post_process%d", class_no);
		f->post_process = config_get_option(c, module_name, conf_name);

                le->data = f;
                list_push_back(ret, le);
        };

        return ret;
out2:
        free(ret);
out1: 
        return NULL;

}

