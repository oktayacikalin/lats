#include <gnome.h>

void
printd ( char *text );

void
quit_lats ( void );

void
refresh_screen ( void );

void
update_screen ( void );

void
set_cursor_for_widget ( GtkWidget *widget, int type );

void
on_new_file1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_paste1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_clear1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_properties1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit0_activate                      (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_MainWindow_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_MainWindow_destroy_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_combo_entry1_activate               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_button9_clicked                     (GtkButton       *button,
                                        gpointer         user_data);


void
on_dirlist_select_row                  (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_imagelist_select_row                (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_homebutton_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_vmbutton1_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_vmbutton2_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_vmbutton3_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_bgcoloruse_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_bgtiles_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_zoomentry_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_zoom100_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_autozoom_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_autozoomwidth_toggled                    (GtkToggleButton *togglebutton,
											 gpointer         user_data);

void
on_autozoomheight_toggled                    (GtkToggleButton *togglebutton,
											  gpointer         user_data);

void
on_keepaspect_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_scrolledwindow6_size_allocate       (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data);

void
on_zoomm25_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_zoomp25_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_zoom100_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_realzoom_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsimagereload_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefsimageradiobutton1_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsimageradiobutton2_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsimageradiobutton3_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsimageradiobutton4_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_bgcolorok_button1_clicked           (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_bgcolorvp_button_press_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);


void
on_prefsmaintogglebutton1_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_imageiconview_size_allocate         (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data);

void
on_packer1_size_allocate               (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data);

void
on_prefsthumbradiobutton1_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsthumbradiobutton2_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsthumbradiobutton3_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsthumbradiobutton4_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsmainentry1_activate            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefsmainentry2_activate            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefsthumbentry1_activate           (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefsthumbentry2_activate           (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefsthumbradiobutton5_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefsthumbradiobutton6_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_iconlist_select_icon                (GnomeIconList   *gnomeiconlist,
                                        gint             arg1,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_imagedisplay_switch_page            (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        gint             page_num,
                                        gpointer         user_data);

void
on_scrolledwindow7_size_allocate       (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data);

void
on_scrolledwindow8_size_allocate       (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data);

gboolean
on_MainWindow_key_press_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

gboolean
on_MainWindow_key_release_event        (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_frame1radiobutton1_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_frame1radiobutton2_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);


gboolean
on_scrolledwindow6_button_press_event  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_scrolledwindow6_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_fullscreenwindow_button_press_event (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_fullscreenwindow_hide               (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fullscreenwindow_show               (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_frame1movebutton1_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_frame1movebutton2_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefsthumbclearbutton_clicked       (GtkButton       *button,
                                        gpointer         user_data);


void
on_prefsthumbupdatebutton_clicked      (GtkButton       *button,
                                        gpointer         user_data);



void
on_toggle_fullscreen1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_toggle_desktop_view1_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_iconlist2_select_icon               (GnomeIconList   *gnomeiconlist,
                                        gint             arg1,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_prefsmainloadbutton_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefsmainsavebutton_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefsmainreset_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefsimagecalibratebutton_clicked   (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobuttoninch_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobuttonmm_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_calibrate_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_calibrate_cancelbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_calibratewindow_show                (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_scrolledwindow6_motion_notify_event (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data);

gboolean
on_fullscreenwindow_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_fullscreenwindow_motion_notify_event
                                        (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data);
