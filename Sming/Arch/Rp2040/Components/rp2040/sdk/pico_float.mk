# pico_float

RUNTIME_INIT_FUNC += \
	__aeabi_float_init

WRAPPED_FUNCTIONS += \
	__aeabi_fadd \
	__aeabi_fdiv \
	__aeabi_fmul \
	__aeabi_frsub \
	__aeabi_fsub \
	__aeabi_cfcmpeq \
	__aeabi_cfrcmple \
	__aeabi_cfcmple \
	__aeabi_fcmpeq \
	__aeabi_fcmplt \
	__aeabi_fcmple \
	__aeabi_fcmpge \
	__aeabi_fcmpgt \
	__aeabi_fcmpun \
	__aeabi_i2f \
	__aeabi_l2f \
	__aeabi_ui2f \
	__aeabi_ul2f \
	__aeabi_f2iz \
	__aeabi_f2lz \
	__aeabi_f2uiz \
	__aeabi_f2ulz \
	__aeabi_f2d \
	sqrtf \
	cosf \
	sinf \
	tanf \
	atan2f \
	expf \
	logf \
	ldexpf \
	copysignf \
	truncf \
	floorf \
	ceilf \
	roundf \
	sincosf \
	asinf \
	acosf \
	atanf \
	sinhf \
	coshf \
	tanhf \
	asinhf \
	acoshf \
	atanhf \
	exp2f \
	log2f \
	exp10f \
	log10f \
	powf \
	powintf \
	hypotf \
	cbrtf \
	fmodf \
	dremf \
	remainderf \
	remquof \
	expm1f \
	log1pf \
	fmaf
