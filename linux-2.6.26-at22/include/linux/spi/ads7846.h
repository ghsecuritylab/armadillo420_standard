/* linux/spi/ads7846.h */

/* Touchscreen characteristics vary between boards and models.  The
 * platform_data for the device's "struct device" holds this information.
 *
 * It's OK if the min/max values are zero.
 */
enum ads7846_filter {
	ADS7846_FILTER_OK,
	ADS7846_FILTER_REPEAT,
	ADS7846_FILTER_IGNORE,
};

struct ads7846_platform_data {
	u16	model;			/* 7843, 7845, 7846. */
	u16	vref_delay_usecs;	/* 0 for external vref; etc */
	u16	vref_mv;		/* external vref value, milliVolts */
	bool	keep_vref_on;		/* set to keep vref on for differential
					 * measurements as well */

	/* Settling time of the analog signals; a function of Vcc and the
	 * capacitance on the X/Y drivers.  If set to non-zero, two samples
	 * are taken with settle_delay us apart, and the second one is used.
	 * ~150 uSec with 0.01uF caps.
	 */
	u16	settle_delay_usecs;

	/* If set to non-zero, after samples are taken this delay is applied
	 * and penirq is rechecked, to help avoid false events.  This value
	 * is affected by the material used to build the touch layer.
	 */
	u16	penirq_recheck_delay_usecs;

	u16	x_plate_ohms;
	u16	y_plate_ohms;

	u16	x_min, x_max;
	u16	y_min, y_max;
	u16	pressure_min, pressure_max;

	u16	debounce_max;		/* max number of additional readings
					 * per sample */
	u16	debounce_tol;		/* tolerance used for filtering */
	u16	debounce_rep;		/* additional consecutive good readings
					 * required after the first two */
	int	(*get_pendown_state)(void);
	int	(*filter_init)	(struct ads7846_platform_data *pdata,
				 void **filter_data);
	int	(*filter)	(void *filter_data, int data_idx, int *val);
	void	(*filter_cleanup)(void *filter_data);

	int 	(*filter_all_vals)(u16 Rt, u16 x, u16 y);

	int	zero_p_ignore_times;

	unsigned	(*vbatt_adjust)(ssize_t v);

	unsigned int	(*calc_rt)(u16 x, u16 y, u16 z1, u16 z2);
};

