#ifndef __CONF_H_
#define __CONF_H_

#include "config.h"	// Various macros
#include "typedefs.h"	// typedefs

/*!
 * @brief Initialise the configuration hive.
 * @param	sync	If true, 'sync' will be called automatically after
 * 			every write.  Otherwise, you will need to call it
 * 			manually when you want changes to the hive to be
 * 			saved to permanent storage.
 * @result The configuration hive can be accessed via the conf_* methods.
 */
void conf_initialise(bool sync);

/*!
 * @brief Read a user setting from the current configuration hive.
 * @param	area	The application-defined area of this configuration
 * 			entry.  For example, "accounts".
 * @param	subarea	The application-defined sub-area of this
 * 			configuration entry.  For example, "eBay".
 * 			This may be NULL if not required, in which case
 * 			the entry is read directly from the area.
 * @param	name	The name of the entry.  For example, "username".
 * @param	def	The default value of the entry, if it has not yet
 * 			been set by the user.  This may be NULL, which
 * 			will then be the return value.
 * @returns The user's current setting for the specified configuration,
 * or <default> if the user has not yet set a value for the entry.
 */
char *conf_read_str(const char *area, const char *subarea,
		    const char *name, char *def);

/*!
 * @brief Set a user setting in the current configuration hive.
 * @param	area	The application-defined area of this configuration
 * 			entry.  For example, "accounts".
 * @param	subarea	The application-defined sub-area of this
 * 			configuration entry.  For example, "eBay".
 * 			This may be NULL if not required, in which case
 * 			the entry is read directly from the area.
 * @param	name	The name of the entry.  For example, "username".
 * @param	val	The new value of the entry.  If this parameter is
 * 			NULL, the entry will become unset.
 * @result The current configuration hive is updated with the specified
 * value for the entry.  If <val> is NULL, the entry will become unset,
 * and future reads for this entry will return the default value.
 */
void conf_write_str(const char *area, const char *subarea,
		    const char *name, char *val);

/*!
 * @brief Read a user setting from the current configuration hive.
 * @param	area	The application-defined area of this configuration
 * 			entry.  For example, "accounts".
 * @param	subarea	The application-defined sub-area of this
 * 			configuration entry.  For example, "eBay".
 * 			This may be NULL if not required, in which case
 * 			the entry is read directly from the area.
 * @param	name	The name of the entry.  For example, "username".
 * @param	def	The default value of the entry, if it has not yet
 * 			been set by the user.
 * @returns The user's current setting for the specified configuration,
 * or <default> if the user has not yet set a value for the entry.
 */
int conf_read_int(const char *area, const char *subarea, const char *name,
		  int def);

/*!
 * @brief Set a user setting in the current configuration hive.
 * @param	area	The application-defined area of this configuration
 * 			entry.  For example, "accounts".
 * @param	subarea	The application-defined sub-area of this
 * 			configuration entry.  For example, "eBay".
 * 			This may be NULL if not required, in which case
 * 			the entry is read directly from the area.
 * @param	name	The name of the entry.  For example, "username".
 * @param	val	The new value of the entry.
 * @result The current configuration hive is updated with the specified
 * value for the entry.
 */
void conf_write_int(const char *area, const char *subarea,
		    const char *name, int val);

/*!
 * @brief Synchronise the current configuration changes.
 * @result The changes made to the current configuration hive are stored
 * permanently.
 * @note This method is called for you automatically if <conf_initialise>
 * was called with 'sync' set to true.  Calling this method if 'sync' is
 * true will be a no-op.
 */
void conf_sync();

/*!
 * @brief Dispose of the current configuration hive.
 * @result All resources related to the configuration hive are freed.
 * Future calls to conf_* methods will fail.
 */
void conf_dispose();

#endif /*!__CONF_H_*/
