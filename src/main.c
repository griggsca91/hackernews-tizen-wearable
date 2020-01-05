/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main.h"

typedef struct appdata {
	Evas_Object *nf;
	Eext_Circle_Surface *circle_surface;
} appdata_s;

typedef struct item_data {
	int index;
	Elm_Object_Item *item;
} item_data;

size_t *article_ids = NULL;
size_t article_ids_length = 0;

void
update_genlist(Evas_Object *genlist);

/* Callback for receiving the response header */
void
header_cb(http_transaction_h transaction, char* header, size_t header_len, void* user_data) {
	printf("header_callback");
}


/* Callback for receiving the response body */
void
body_cb(http_transaction_h transaction, char* body, size_t size, size_t nmemb, void* user_data) {
	GError *error = NULL;

	JsonNode *result = json_from_string(body, &error);
	JsonArray *article_ids_json_array = json_node_get_array(result);

	article_ids_length = json_array_get_length(article_ids_json_array);
	JsonNode *first_value = json_array_get_element(article_ids_json_array, 0);

	dlog_print(DLOG_DEBUG, "SRBD", "Size: %d", article_ids_length);
	dlog_print(DLOG_DEBUG, "SRBD", "Article ID: %lld", json_node_get_int(first_value));

	article_ids = (size_t *) malloc(sizeof(size_t) * article_ids_length);
	for (int i = 0; i < article_ids_length; i++) {
		JsonNode *article_id = json_array_get_element(article_ids_json_array, i);
		article_ids[i] = json_node_get_int(article_id);
	}


	printf("parsed body");

	Evas_Object *genlist = (Evas_Object *) user_data;



	elm_genlist_clear(genlist);
	update_genlist(genlist);

}


/* Callback for monitoring data uploads */
void
uploaded_cb(http_transaction_h transaction, char* body, int recommended_chunk, void* user_data) {
	printf("uploaded callback");
}


/* Callback for monitoring transaction completions */
void
completed_cb(http_transaction_h transaction, char* body, void* user_data) {
	printf("completed callback");
}


/* Callback for monitoring transaction abortions */
void
aborted_cb(http_transaction_h transaction, int reason, void* user_data) {
	printf("aborted callback");
}


void
get_top_stories(Evas_Object *genlist) {
	int ret = HTTP_ERROR_NONE;

	ret = http_init();
	if (ret != HTTP_ERROR_NONE)
	    printf("http_init failed: %d", ret);

	http_session_h session = NULL;

	ret = http_session_create(HTTP_SESSION_MODE_NORMAL, &session);
	if (ret != HTTP_ERROR_NONE)
	    printf("http_session_create failed: %d", ret);

	bool auto_redirect = true;

	ret = http_session_set_auto_redirection(session, auto_redirect);
	if (ret != HTTP_ERROR_NONE)
	    printf("http_session_set_auto_redirection failed: %d", ret);


	http_transaction_h transaction = NULL;
	ret = http_session_open_transaction(session, HTTP_METHOD_GET, &transaction);
	if (ret != HTTP_ERROR_NONE)
	    printf("http_session_open_transaction failed: %d", ret);


	http_transaction_set_received_header_cb(transaction, header_cb, NULL);
	http_transaction_set_received_body_cb(transaction, body_cb, (void *) genlist);
	http_transaction_set_uploaded_cb(transaction, uploaded_cb, NULL);
	http_transaction_set_completed_cb(transaction, completed_cb, NULL);
	http_transaction_set_aborted_cb(transaction, aborted_cb, NULL);

	char uri[1024] = "https://hacker-news.firebaseio.com/v0/topstories.json";

	ret = http_transaction_request_set_uri(transaction, uri);
	if (ret != HTTP_ERROR_NONE)
	    printf("http_transaction_request_set_uri failed: %d", ret);

	http_method_e method = HTTP_METHOD_GET;

	ret = http_transaction_request_set_method(transaction, method);
	if (ret != HTTP_ERROR_NONE)
	    printf("http_transaction_request_set_method failed: %d", ret);

	http_version_e version = HTTP_VERSION_1_1;

	ret = http_transaction_request_set_version(transaction, version);
	if (ret != HTTP_ERROR_NONE)
	    printf("http_transaction_request_set_version failed: %d", ret);

	ret = http_transaction_submit(transaction);
	if (ret != HTTP_ERROR_NONE)
	    printf("http_transaction_submit failed: %d", ret);

}



/*
 * @brief Function will be operated when window deletion is requested
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

/*
 * @brief Function to get string on genlist item's text part
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] part The name of text part
 * @param[out] char* A string with the characters to use as genlist item's text part
 */
static char*
gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	item_data *id = data;
	int index = id->index;

	if (index == 0)
		snprintf(buf, 1023, "%s", "Email Test");
	else if (index == 1)
		snprintf(buf, 1023, "%s", "circle@tizen.com");
	else if (!strcmp(part, "elm.text") && (index - 2) <= article_ids_length)
		snprintf(buf, 1023, "%zu", article_ids[index - 2]);
	else if (!strcmp(part, "elm.text.1"))
		snprintf(buf, 1023, "%s", "Re: Long time no see");
	else
		snprintf(buf, 1023, "%s", "Hello~! how have you been?");

	return strdup(buf);
}

/*
 * @brief Function will be operated when genlist is deleted.
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 */
static void
gl_del(void *data, Evas_Object *obj)
{
	item_data *id = data;
	if (id) {
		free(id);
	}
}

/*
 * @brief Function to create label
 * @param[in] ad The data structure to manage gui object
 * @param[out] Evas_Object The label which is created
 */
static Evas_Object*
create_label(appdata_s *ad, Elm_Object_Item *it)
{
	Evas_Object *label;
	char buf[PATH_MAX];

	label = elm_label_add(ad->nf);

	//label creates for text view.
	snprintf(buf, sizeof(buf), " %s %s", elm_object_item_part_text_get(it, "elm.text.1"),
										elm_object_item_part_text_get(it, "elm.text.2"));
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, buf);
	evas_object_show(label);

	return label;
}

/*
 * @brief Function will be operated when genlist item is selected.
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void
gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *label;
	Elm_Object_Item *it = event_info;
	appdata_s *ad = data;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	label = create_label(ad, it);
	//View changed to text detail view.
	elm_naviframe_item_push(ad->nf, elm_object_item_part_text_get(it, "elm.text"), NULL, NULL, label, NULL);

	return;
}

/*
 * @brief Function to create genlist
 * @param[in] ad The data structure to manage gui object
 * @param[out] Evas_Object The genlist which is created
 */
static Evas_Object *
create_genlist(appdata_s *ad)
{
	int i;
	Evas_Object *genlist, *circle_genlist;
	Elm_Object_Item *item;

	//Added 2 genlist item class for title, groupindex, items.
	Elm_Genlist_Item_Class *tic = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *gic = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *iic = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *pic = elm_genlist_item_class_new();

	tic->item_style = "title_with_groupindex";
	tic->func.text_get = gl_text_get;
	tic->func.content_get = NULL;
	tic->func.del = gl_del;

	gic->item_style = "groupindex";
	gic->func.text_get = gl_text_get;
	gic->func.content_get = NULL;
	gic->func.del = gl_del;

	iic->item_style = "3text";
	iic->func.text_get = gl_text_get;
	iic->func.content_get = NULL;
	iic->func.del = gl_del;

	pic->item_style = "padding";

	genlist = elm_genlist_add(ad->nf);
	//Eext genlist add to given genlist object. for draw circular scroll bar.
	circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);

	//This genlist is not scrolled to X axis.
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	//Rotary event activated for genlist to get rotary event.
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);

	for (i = 0; i < article_ids_length + 2; i++) {
		item_data *id = calloc(sizeof(item_data), 1);
		id->index = i;

		if (i == 0) {
			item = elm_genlist_item_append(
											genlist,            // genlist object
											tic,                // item class
											id,                 // data
											NULL,
											ELM_GENLIST_ITEM_GROUP,
											NULL,
											NULL);
			//Title item in only for display.
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else if (i == 1) {
		    item = elm_genlist_item_append(
											genlist,            // genlist object
											gic,                // item class
											id,                 // data
											NULL,
											ELM_GENLIST_ITEM_GROUP,
											NULL,
											NULL);
			//groupindex item in only for display.
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else
		    item = elm_genlist_item_append(
											genlist,            // genlist object
											iic,                // item class
											id,                 // data
											NULL,
											ELM_GENLIST_ITEM_NONE,
											NULL,
											NULL);

		id->item = item;
	}
	// Padding Item Here
	elm_genlist_item_append(genlist, pic, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);


	elm_genlist_item_class_free(tic);
	elm_genlist_item_class_free(gic);
	elm_genlist_item_class_free(iic);
	elm_genlist_item_class_free(pic);
	evas_object_show(genlist);

	return genlist;
}


/*
 * @brief Function to create genlist
 * @param[in] ad The data structure to manage gui object
 * @param[out] Evas_Object The genlist which is created
 */
void
update_genlist(Evas_Object *genlist)
{
	int i;
	Evas_Object *circle_genlist;
	Elm_Object_Item *item;

	//Added 2 genlist item class for title, groupindex, items.
	Elm_Genlist_Item_Class *tic = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *gic = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *iic = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *pic = elm_genlist_item_class_new();

	tic->item_style = "title_with_groupindex";
	tic->func.text_get = gl_text_get;
	tic->func.content_get = NULL;
	tic->func.del = gl_del;

	gic->item_style = "groupindex";
	gic->func.text_get = gl_text_get;
	gic->func.content_get = NULL;
	gic->func.del = gl_del;

	iic->item_style = "3text";
	iic->func.text_get = gl_text_get;
	iic->func.content_get = NULL;
	iic->func.del = gl_del;

	pic->item_style = "padding";


	for (i = 0; i < article_ids_length + 2; i++) {
		item_data *id = calloc(sizeof(item_data), 1);
		id->index = i;

		if (i == 0) {
			item = elm_genlist_item_append(
											genlist,            // genlist object
											tic,                // item class
											id,                 // data
											NULL,
											ELM_GENLIST_ITEM_GROUP,
											NULL,
											NULL);
			//Title item in only for display.
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else if (i == 1) {
		    item = elm_genlist_item_append(
											genlist,            // genlist object
											gic,                // item class
											id,                 // data
											NULL,
											ELM_GENLIST_ITEM_GROUP,
											NULL,
											NULL);
			//groupindex item in only for display.
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else
		    item = elm_genlist_item_append(
											genlist,            // genlist object
											iic,                // item class
											id,                 // data
											NULL,
											ELM_GENLIST_ITEM_NONE,
											NULL,
											NULL);

		id->item = item;
	}
	// Padding Item Here
	elm_genlist_item_append(genlist, pic, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);


	elm_genlist_item_class_free(tic);
	elm_genlist_item_class_free(gic);
	elm_genlist_item_class_free(iic);
	elm_genlist_item_class_free(pic);
//	evas_object_show(genlist);
}


/*
 * @brief Function will be operated when naviframe pop its own item
 * @param[in] data The data to be passed to the callback function
 * @param[in] it The item to be popped from naviframe
 */
static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	ui_app_exit();
	return EINA_FALSE;
}

/*
 * @brief Function to create gui object
 * @param[in] ad The data structure to manage gui object
 */
static void
create_main_view(appdata_s *ad)
{
	Elm_Object_Item *nf_it;
	Evas_Object *genlist;

	genlist = create_genlist(ad);

	get_top_stories(genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}



/*
 * @brief Function to create base gui structure
 * @param[in] ad The data structure to manage gui object
 */
static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	Evas_Object *win, *conform;
	win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(win, EINA_TRUE);

	evas_object_smart_callback_add(win, "delete,request", win_delete_request_cb, NULL);

	/* Conformant */
	conform = elm_conformant_add(win);

	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, conform);
	evas_object_show(conform);

	/* Naviframe */
	ad->nf = elm_naviframe_add(conform);
	elm_object_content_set(conform, ad->nf);
	ad->circle_surface = eext_circle_surface_naviframe_add(ad->nf);

	create_main_view(ad);

	eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);

	/* Show window after base gui is set up */
	evas_object_show(win);
}

/*
 * @brief Hook to take necessary actions before main event loop starts
 * Initialize UI resources and application's data
 * If this function return true, the main loop of application starts
 * If this function return false, the application is terminated
 * @param[in] user_data The data to be passed to the callback function
 */
static bool app_create(void *user_data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = user_data;

	create_base_gui(ad);

	return true;
}

/*
 * @brief This callback function is called when another application
 * sends the launch request to the application
 * @param[in] app_control The handle to the app_control
 * @param[in] user_data The data to be passed to the callback function
 */
static void app_control(app_control_h app_control, void *user_data)
{
	/* Handle the launch request. */
}

/*
 * @brief This callback function is called each time
 * the application is completely obscured by another application
 * and becomes invisible to the user
 * @param[in] user_data The data to be passed to the callback function
 */
static void app_pause(void *user_data)
{
	/* Take necessary actions when application becomes invisible. */
}

/*
 * @brief This callback function is called each time
 * the application becomes visible to the user
 * @param[in] user_data The data to be passed to the callback function
 */
static void app_resume(void *user_data)
{
	/* Take necessary actions when application becomes visible. */
}

/*
 * @brief This callback function is called once after the main loop of the application exits
 * @param[in] user_data The data to be passed to the callback function
 */
static void app_terminate(void *user_data)
{
	/* Release all resources. */
}

/*
 * @brief This function will be called when the language is changed
 * @param[in] event_info The system event information
 * @param[in] user_data The user data to be passed to the callback function
 */
static void ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

/*
 * @brief This function will be called when the orientation is changed
 * @param[in] event_info The system event information
 * @param[in] user_data The user data to be passed to the callback function
 */
static void ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

/*
 * @brief This function will be called when the region is changed
 * @param[in] event_info The system event information
 * @param[in] user_data The user data to be passed to the callback function
 */
static void ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

/*
 * @brief This function will be called when the battery is low
 * @param[in] event_info The system event information
 * @param[in] user_data The user data to be passed to the callback function
 */
static void ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

/*
 * @brief This function will be called when the memory is low
 * @param[in] event_info The system event information
 * @param[in] user_data The user data to be passed to the callback function
 */
static void ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

/*
 * @brief Main function of the application
 * @param[in] argc The argument count
 * @param[in] argv The argument vector
 */
int main(int argc, char *argv[])
{
	appdata_s ad = {0, };
	int ret;

	ui_app_lifecycle_callback_s event_callback = {0, };
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);

	return ret;
}
