#ifndef __NVRW_INTERFACE_H__
#define __NVRW_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

	#define MD5_DIGEST_LENGTH 16
	#define UPDATE_FILE_MAX 10

	/**
	 * @brief used for nvinfo_t->update_flag
	 */
	typedef enum flag {
		/**
		 * @brief no new version
		 */
		FLAG_NONUPDATE = 0,
		/**
		 * @brief has new version
		 */
		FLAG_UPDATE,
	} flag_t;

	/**
	 * @brief used for nvinfo_t->update_process
	 */
	typedef enum process {
		/**
		 * @brief process1:
		 *   update_once: download updaterfs+kernel
		 *   update_block: download updaterfs+kernel
		 */
		PROCESS_1 = 1,
		/**
		 * @brief process2:
		 *   update_once: download and update updaterfs+kernel+appfs
		 *   update_block: update updaterfs+kernel
		 */
		PROCESS_2,
		/**
		 * @brief process3:
		 *   update_once: Reserve
		 *   update_block: download & update appfs.
		 */
		PROCESS_3,
		/**
		 * @brief update done.
		 */
		PROCESS_DONE,
	} process_t;

	/**
	 * @brief used for nvinfo_t->update_method->method
	 */
	typedef enum method {
		/**
		 * @brief download new package, then update all partions in ramdisk, need reboot once only.
		 */
		UPDATE_ONCE = 1,
		/**
		 * @brief update partion one by one. need reboot several times.
		 */
		UPDATE_TIMES,
	} method_t;

	/**
	 * @brief ota file info
	 */
	struct otafile_info {
		/**
		 * @brief file name
		 */
		char name[32];
		/**
		 * @brief file md5
		 */
		char md5[MD5_DIGEST_LENGTH * 2 + 1];
		/**
		 * @brief file target
		 */
		char blkdev[16];
		/**
		 * @brief update package src
		 */
		char location[32];
		/**
		 * @brief offset on location
		 */
		long long offset;
		/**
		 * @brief file size
		 */
		long long size;
	} __attribute__ ((aligned(8)));
	/**
	 * @brief nv info
	 */
	typedef struct nv_info {
		/**
		 * @brief magic for start flag
		 */
		char start_magic[4];
		/**
		 * @brief current version, format: 1.7.0
		 */
		char current_version[18];
		/**
		 * @brief new version, format: 1.7.1
		 */
		char update_version[18];
		/**
		 * @brief have update flag.
		 */
		flag_t update_flag;
		/**
		 * @brief update procedure, ONLY used for spinor update method now.
		 */
		process_t update_process;
		/**
		 * @brief update method
		 */
		struct {
			/**
			 * @brief method.
			 */
			method_t method;
			/**
			 * @brief nand, spinand, spinor, mmc
			 */
			unsigned char storage[8];
			/**
			 * @brief update package src
			 */
			unsigned char location[32];
		} update_method; // TODO: expand method.
		/**
		 * @brief product name, will be append to the tail of url when download update package.
		 */
		unsigned char product[32];
		/**
		 * @brief update server url.
		 */
		unsigned char url[4096]; // TODO: url size
		/**
		 * @brief ota file count.
		 */
		int otafile_cnt;
		/**
		 * @brief ota files.
		 */
		struct otafile_info files[UPDATE_FILE_MAX];
		/**
		 * @brief magic for end flag.
		 */
		char end_magic[4];
	} nvinfo_t;

	/**
	 * @brief get nv info
	 *
	 * @return return nv info on success(Need free it after use), NULL otherwise.
	 */
	extern nvinfo_t *mozart_nv_get_nvinfo(void);

	/**
	 * @brief get nv rodata.
	 *
	 * @param size [in] rodata bytes to be get
	 *
	 * @return return rodata on success(Need free it after use), NULL otherwise.
	 */
	extern void *mozart_nv_get_rodata(int size);

	/**
	 * @brief write nv info
	 *
	 * @param nvinfo [in] nv info to be write
	 *
	 * @return return 0 On success, -1 otherwiase.
	 */
	extern int mozart_nv_set_nvinfo(nvinfo_t *nvinfo);

	/**
	 * @brief nv info read/write lock
	 *
	 * @return return lock On success, -1 otherwiase.
	 */
	extern int mozart_nv_lock(void);

	/**
	 * @brief unlock nv info read/write lock.
	 *
	 * @param lock
	 *
	 * @return return 0 On success, -1 otherwiase.
	 */
	extern int mozart_nv_unlock(int lock);

#ifdef __cplusplus
}
#endif

#endif /* __NVRW_INTERFACE_H__ */
