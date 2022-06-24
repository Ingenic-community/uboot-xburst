#include <config.h>
#include <common.h>
#include <linux/err.h>
#include <linux/list.h>
#include <regulator.h>
#include <ingenic_soft_i2c.h>
#ifndef CONFIG_SPL_BUILD
#include <power/sm5007.h>
#endif

enum regulator_id {
	SM5007_ID_BUCK1 = 0,
	SM5007_ID_BUCK1_DVS,
	SM5007_ID_BUCK2,
	SM5007_ID_BUCK3,
	SM5007_ID_BUCK4,
	SM5007_ID_LDO1,
	SM5007_ID_LDO2,
	SM5007_ID_LDO3,
	SM5007_ID_LDO4,
	SM5007_ID_LDO5,
	SM5007_ID_LDO6,
	SM5007_ID_LDO7,
	SM5007_ID_LDO8,
	SM5007_ID_LDO9,
	SM5007_ID_PS1,
	SM5007_ID_PS2,
	SM5007_ID_PS3,
	SM5007_ID_PS4,
	SM5007_ID_PS5,
};

static const unsigned int sm5703_buck_output_list[] = {0 * 1000, };

static const unsigned int sm5703_ldo_output_list[] = {
	800 * 1000,
	900 * 1000,
	1000 * 1000,
	1100 * 1000,
	1200 * 1000,
	1500 * 1000,
	1800 * 1000,
	2000 * 1000,
	2100 * 1000,
	2500 * 1000,
	2600 * 1000,
	2700 * 1000,
	2800 * 1000,
	2900 * 1000,
	3000 * 1000,
	3300 * 1000,
};

struct SM5007_regulator {
	int id;
	int sleep_id;
	/* Regulator register address.*/
	u8 reg_en_reg;
	u8 en_bit;
	u8 vout_reg;
	u8 vout_mask;
	u8 vout_shift;
	u8 reg_slot_pwr_reg;
	u8 lpm_reg;
	u8 lpm_mask;
	u8 lpm_shift;

	/* chip constraints on regulator behavior */
	int min_uV;
	int max_uV;
	int step_uV;
	int nsteps;

	/* regulator specific turn-on delay */
	u16 delay;

	/* used by regulator core */
	struct regulator desc;

	/* Device */
	struct device *dev;
};

static struct i2c sm5007_i2c;
static struct i2c *i2c;

static inline int __sm5007_read(unsigned char addr, u8 reg, uint8_t *val)
{
	int ret;

	ret = i2c_read(i2c, addr, reg, 1, val, 1);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

static inline int __sm5007_write(unsigned char addr, u8 reg, uint8_t *val)
{
	int ret;

	ret = i2c_write(i2c, addr, reg, 1, val, 1);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

int sm5007_set_bits(unsigned char addr, u8 reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = __sm5007_read(addr, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = __sm5007_write(addr, reg, &reg_val);
	}
out:
	return ret;
}

int sm5007_clr_bits(unsigned char addr, u8 reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = __sm5007_read(addr, reg, &reg_val);
	if (ret)
		goto out;

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = __sm5007_write(addr, reg, &reg_val);
	}
out:
	return ret;
}

int sm5007_read(unsigned char addr, u8 reg, uint8_t *val)
{
	int ret = 0;

	ret = __sm5007_read(addr, reg, val);

	return ret;
}

int sm5007_write(unsigned char addr, u8 reg, uint8_t val)
{
	int ret = 0;

	ret = __sm5007_write(addr, reg, &val);

	return ret;
}

void *sm5007_rdev_get_drvdata(struct regulator *rdev)
{
	return rdev->reg_data;
}

int sm5007_rdev_set_drvdata(struct regulator *rdev, void *data)
{
	rdev->reg_data = data;
	return 0;
}

static int SM5007_reg_is_enabled(struct regulator *rdev)
{
	struct SM5007_regulator *ri =
		(struct SM5007_regulator *) sm5007_rdev_get_drvdata(rdev);
	unsigned char addr = SM5007_PMU_ADDR;
	uint8_t control;
	int ret = 0, is_enable = 0, tem_enable = 0;

	ret = sm5007_read(addr, ri->reg_en_reg, &control);
	//dong
	//Need to add PWRSTM Pin condition
	//If PWRSTM pin is high, condition is disable in case of en_bit 1 or 2.
	switch (ri->id) {
		case SM5007_ID_BUCK1 ... SM5007_ID_BUCK1_DVS:
			tem_enable = ((control  >> ri->en_bit) & 0x3);
			is_enable = (tem_enable == 0x03);
			break;
		case SM5007_ID_BUCK2 ... SM5007_ID_LDO1:
		case SM5007_ID_LDO3 ... SM5007_ID_LDO5:
		case SM5007_ID_LDO8:
		case SM5007_ID_PS1 ... SM5007_ID_PS5:
			tem_enable = ((control  >> ri->en_bit) & 0x3);
			is_enable = ((tem_enable == 0x00) ? 0 :
					(tem_enable == 0x02) ? 1 :
					(tem_enable == 0x03) ? 1 : 0);
			break;
		case SM5007_ID_LDO2:
		case SM5007_ID_LDO6 ... SM5007_ID_LDO7:
		case SM5007_ID_LDO9:
			tem_enable = ((control >> ri->en_bit) & 0x1);
			is_enable = tem_enable;
			break;
		default:
			break;
	}

	return is_enable;
}

static int SM5007_reg_enable(struct regulator *rdev)
{
	struct SM5007_regulator *ri =
		(struct SM5007_regulator *) sm5007_rdev_get_drvdata(rdev);
	unsigned char addr = SM5007_PMU_ADDR;
	int ret = 0;

	switch (ri->id) {
		case SM5007_ID_BUCK1 ... SM5007_ID_BUCK1_DVS:
			ret = sm5007_set_bits(addr, ri->reg_en_reg, (0x3 << ri->en_bit));
			break;
		case SM5007_ID_BUCK2 ... SM5007_ID_LDO1:
		case SM5007_ID_LDO3 ... SM5007_ID_LDO5:
		case SM5007_ID_LDO8:
		case SM5007_ID_PS1 ... SM5007_ID_PS5:
			ret = sm5007_set_bits(addr, ri->reg_en_reg, (0x3 << ri->en_bit));
			break;
		case SM5007_ID_LDO2:
		case SM5007_ID_LDO6 ... SM5007_ID_LDO7:
		case SM5007_ID_LDO9:
			ret = sm5007_set_bits(addr, ri->reg_en_reg, (0x1 << ri->en_bit));
			break;
		default:
			break;
	}

	return ret;
}

static int SM5007_reg_disable(struct regulator *rdev)
{
	struct SM5007_regulator *ri =
		(struct SM5007_regulator *) sm5007_rdev_get_drvdata(rdev);
	unsigned char addr = SM5007_PMU_ADDR;
	int ret = 0;

	switch (ri->id) {
		case SM5007_ID_BUCK1 ... SM5007_ID_BUCK1_DVS:
			ret = sm5007_clr_bits(addr, ri->reg_en_reg, (0x3 << ri->en_bit));
			break;
		case SM5007_ID_BUCK2 ... SM5007_ID_LDO1:
		case SM5007_ID_LDO3 ... SM5007_ID_LDO5:
		case SM5007_ID_LDO8:
		case SM5007_ID_PS1 ... SM5007_ID_PS5:
			ret = sm5007_clr_bits(addr, ri->reg_en_reg, (0x3 << ri->en_bit));
			break;
		case SM5007_ID_LDO2:
		case SM5007_ID_LDO6 ... SM5007_ID_LDO7:
		case SM5007_ID_LDO9:
			ret = sm5007_clr_bits(addr, ri->reg_en_reg, (0x1 << ri->en_bit));
			break;
		default:
			break;
	}

	return ret;
}

static int __SM5007_set_voltage(unsigned char addr,
		struct SM5007_regulator *ri, int min_uV, int max_uV)
{
	int vsel = 0;
	int ret = 0;
	int i = 0;
	uint8_t vout_val, read_val;

	switch (ri->id) {
		case SM5007_ID_BUCK1 ... SM5007_ID_BUCK1_DVS:
		case SM5007_ID_BUCK4:
			if ((min_uV < ri->min_uV) || (max_uV > ri->max_uV))
				return -EDOM;

			vsel = (min_uV - ri->min_uV + ri->step_uV - 1) / ri->step_uV;
			if (vsel > ri->nsteps)
				return -EDOM;

			ret = sm5007_read(addr, ri->vout_reg, &read_val);

			vout_val = (read_val & ~ri->vout_mask) | (vsel << ri->vout_shift);
			ret = sm5007_write(addr, ri->vout_reg, vout_val);
			sm5007_read(addr, ri->vout_reg, &read_val);

			break;
		case SM5007_ID_LDO1 ... SM5007_ID_LDO2:
		case SM5007_ID_LDO7 ... SM5007_ID_LDO9:
			if ((min_uV < ri->min_uV) || (max_uV > ri->max_uV))
				return -EDOM;

			for (i = 0; i < 16; i++) {
				if (sm5703_ldo_output_list[i] > min_uV) {
					vsel = i - 1;
					break;
				}
			}
			if (vsel < 0)
				vsel = 0;

			ret = sm5007_read(addr, ri->vout_reg, &read_val);

			vout_val = (read_val & ~ri->vout_mask) | (vsel << ri->vout_shift);
			ret = sm5007_write(addr, ri->vout_reg, vout_val);
			sm5007_read(addr, ri->vout_reg, &read_val);

			break;
		case SM5007_ID_BUCK2 ... SM5007_ID_BUCK3:
		case SM5007_ID_LDO3 ... SM5007_ID_LDO6:
		case SM5007_ID_PS1 ... SM5007_ID_PS5:
			ret = 0;
			break;
		default:
			break;
	}

	return ret;
}

static int SM5007_set_voltage(struct regulator *rdev, int min_uV, int max_uV)
{
	struct SM5007_regulator *ri =
		(struct SM5007_regulator *) sm5007_rdev_get_drvdata(rdev);

	return __SM5007_set_voltage(SM5007_PMU_ADDR, ri, min_uV, max_uV);
}

static struct regulator_ops SM5007_ops = { .set_voltage = SM5007_set_voltage,
	.enable = SM5007_reg_enable, .disable = SM5007_reg_disable,
	.is_enabled = SM5007_reg_is_enabled, };

#define sm5007_rails(_name) "SM5007_"#_name

#define SM5007_REG_BUCK(_id, _en_reg, _en_bit, _slot_pwr_reg, _vout_reg, _vout_mask, _vout_shift,\
		_min_mv, _max_mv, _step_uV, _nsteps, _ops, _delay, _lpm_reg, _lpm_mask, _lpm_shift) \
{								\
	.reg_en_reg	= _en_reg,				\
	.en_bit		= _en_bit,				\
	.reg_slot_pwr_reg	= _slot_pwr_reg,			\
	.vout_reg	= _vout_reg,				\
	.vout_mask	= _vout_mask,				\
	.vout_shift	= _vout_shift,				\
	.min_uV		= _min_mv * 1000,			\
	.max_uV		= _max_mv * 1000,			\
	.step_uV	= _step_uV,				\
	.nsteps		= _nsteps,				\
	.delay		= _delay,				\
	.lpm_reg = _lpm_reg,            \
	.lpm_mask = _lpm_mask,            \
	.lpm_shift = _lpm_shift,            \
	.id		= SM5007_ID_##_id,			\
	.desc = {						\
		.name = sm5007_rails(_id),			\
		.id = SM5007_ID_##_id,			\
		.n_voltages = _nsteps,				\
		.ops = &_ops,					\
		.min_uV		= _min_mv * 1000,			\
		.max_uV		= _max_mv * 1000,			\
	},							\
}

#define SM5007_REG_LDO(_id, _en_reg, _en_bit, _slot_pwr_reg, _vout_reg, _vout_mask, _vout_shift,\
		_min_mv, _max_mv, _step_uV, _nsteps, _ops, _delay, _lpm_reg, _lpm_mask, _lpm_shift) \
{								\
	.reg_en_reg	= _en_reg,				\
	.en_bit		= _en_bit,				\
	.reg_slot_pwr_reg = _slot_pwr_reg,			\
	.vout_reg	= _vout_reg,				\
	.vout_mask	= _vout_mask,				\
	.vout_shift	= _vout_shift,				\
	.min_uV		= _min_mv * 1000,			\
	.max_uV		= _max_mv * 1000,			\
	.step_uV	= _step_uV,				\
	.nsteps		= _nsteps,				\
	.delay		= _delay,				\
	.lpm_reg = _lpm_reg,            \
	.lpm_mask = _lpm_mask,            \
	.lpm_shift = _lpm_shift,            \
	.id		= SM5007_ID_##_id,			\
	.desc = {						\
		.name = sm5007_rails(_id),			\
		.id = SM5007_ID_##_id,			\
		.n_voltages = _nsteps,				\
		.ops = &_ops,					\
		.min_uV		= _min_mv * 1000,			\
		.max_uV		= _max_mv * 1000,			\
	},							\
}

#define SM5007_REG_PS(_id, _en_reg, _en_bit, _slot_pwr_reg, _vout_reg, _vout_mask, _vout_shift,\
		_min_mv, _max_mv, _step_uV, _nsteps, _ops, _delay, _lpm_reg, _lpm_mask, _lpm_shift) \
{								\
	.reg_en_reg	= _en_reg,				\
	.en_bit		= _en_bit,				\
	.reg_slot_pwr_reg = _slot_pwr_reg,			\
	.vout_reg	= _vout_reg,				\
	.vout_mask	= _vout_mask,				\
	.vout_shift	= _vout_shift,				\
	.min_uV		= _min_mv * 1000,			\
	.max_uV		= _max_mv * 1000,			\
	.step_uV	= _step_uV,				\
	.nsteps		= _nsteps,				\
	.delay		= _delay,				\
	.lpm_reg = _lpm_reg,            \
	.lpm_mask = _lpm_mask,            \
	.lpm_shift = _lpm_shift,            \
	.id		= SM5007_ID_##_id,			\
	.desc = {						\
		.name = sm5007_rails(_id),			\
		.id = SM5007_ID_##_id,			\
		.n_voltages = _nsteps,				\
		.ops = &_ops,					\
		.min_uV		= _min_mv * 1000,			\
		.max_uV		= _max_mv * 1000,			\
	},							\
}

static struct SM5007_regulator SM5007_regulator[] = {
	SM5007_REG_BUCK(BUCK1, 0x30, 0, 0x30, 0x31, 0x3F, 0x00,
			700, 1300, 12500, 0x30, SM5007_ops, 500, 0x46, 0x01, 0x00),
	SM5007_REG_BUCK(BUCK1_DVS, 0x30, 0, 0x30, 0x32, 0x3F, 0x00,
			700, 1300, 12500, 0x30, SM5007_ops, 500, 0x46, 0x01, 0x00),
	SM5007_REG_BUCK(BUCK2, 0x33, 0, 0x33, 0xFF, 0xFF, 0x00,
			1200, 1200, 0, 0x0, SM5007_ops, 500, 0x46, 0x02, 0x01),
	SM5007_REG_BUCK(BUCK3, 0x42, 0, 0x42, 0xFF, 0xFF, 0x00,
			1800, 1800, 0, 0x0, SM5007_ops, 500, 0x46, 0x04, 0x02),
	SM5007_REG_BUCK(BUCK4, 0x42, 1, 0x42, 0x34, 0x3F, 0x00,
			1800, 3300, 50000, 0x1E, SM5007_ops, 500, 0x46, 0x08, 0x03),
	SM5007_REG_LDO(LDO1, 0x35, 0, 0x35, 0x3A, 0x0F, 0x00,
			800, 3300, 0, 0x0F, SM5007_ops, 500, 0x43, 0x03, 0x00),
	SM5007_REG_LDO(LDO2, 0x42, 2, 0x42, 0x3A, 0xF0, 0x04,
			800, 3300, 0, 0x0F, SM5007_ops, 500, 0x43, 0x0C, 0x02),
	SM5007_REG_LDO(LDO3, 0x36, 0, 0x36, 0xFF, 0xFF, 0x00,
			3000, 3000, 0, 0, SM5007_ops, 500, 0x43, 0x30, 0x04),
	SM5007_REG_LDO(LDO4, 0x37, 0, 0x37, 0xFF, 0xFF, 0x00,
			2800, 2800, 0, 0, SM5007_ops, 500, 0x43, 0xC0, 0x06),
	SM5007_REG_LDO(LDO5, 0x38, 0, 0x38, 0xFF, 0xFF, 0x00,
			2500, 2500, 0, 0, SM5007_ops, 500, 0x44, 0x03, 0x00),
	SM5007_REG_LDO(LDO6, 0x42, 3, 0x42, 0xFF, 0xFF, 0x00,
			1800, 1800, 0, 0, SM5007_ops, 500, 0x44, 0x0C, 0x02),
	SM5007_REG_LDO(LDO7, 0x42, 4, 0x42, 0x3B, 0x0F, 0x00,
			800, 3300, 0, 0, SM5007_ops, 500, 0x44, 0x30, 0x04),
	SM5007_REG_LDO(LDO8, 0x39, 0, 0x39, 0x3B, 0xF0, 0x04,
			800, 3300, 0, 0, SM5007_ops, 500, 0x44, 0xC0, 0x06),
	SM5007_REG_LDO(LDO9, 0x42, 5, 0x42, 0x3C, 0x0F, 0x00,
			800, 3300, 0, 0, SM5007_ops, 500, 0x45, 0x03, 0x00),
	SM5007_REG_PS(PS1, 0x3D, 0, 0x3D, 0x31, 0x3F, 0x00,
			1100, 1100, 12500, 0x30, SM5007_ops, 500, 0x00, 0x00, 0x00),
	SM5007_REG_PS(PS2, 0x3E, 0, 0x3E, 0xFF, 0xFF, 0x00,
			1200, 1200, 0, 0x0, SM5007_ops, 500, 0x00, 0x00, 0x00),
	SM5007_REG_PS(PS3, 0x3F, 0, 0x3F, 0xFF, 0xFF, 0x00,
			1800, 1800, 0, 0x0, SM5007_ops, 500, 0x00, 0x00, 0x00),
	SM5007_REG_PS(PS4, 0x40, 0, 0x40, 0xFF, 0xFF, 0x00,
			3000, 3000, 0, 0, SM5007_ops, 500, 0x00, 0x00, 0x00),
	SM5007_REG_PS(PS5, 0x41, 0, 0x41, 0xFF, 0xFF, 0x00,
			2800, 2800, 0, 0, SM5007_ops, 500, 0x00, 0x00, 0x00),
};

int sm5007_regulator_init(void)
{

	int ret,i;
	sm5007_i2c.scl = CONFIG_SM5007_I2C_SCL;
	sm5007_i2c.sda = CONFIG_SM5007_I2C_SDA;
	i2c = &sm5007_i2c;
	i2c_init(i2c);

	ret = i2c_probe(i2c, SM5007_PMU_ADDR);
	if (ret) {
		printf("probe sm5007 error, i2c addr ox%x\n", SM5007_PMU_ADDR);
		return -EIO;
	}
	for (i = 0; i < ARRAY_SIZE(SM5007_regulator); i++) {
		ret = regulator_register(&SM5007_regulator[i].desc, NULL);
		sm5007_rdev_set_drvdata(&SM5007_regulator[i].desc, &SM5007_regulator[i]);
		if (ret)
			printf("%s regulator_register error\n",
					SM5007_regulator[i].desc.name);
	}

	return 0;
}

void sm5007_shutdown(void)
{
	sm5007_write(SM5007_PMU_ADDR, SM5007_CNTL2, SM5007_GLOBAL_SHUTDOWN);
}

#ifdef CONFIG_SPL_BUILD
int spl_regulator_set_voltage(enum regulator_outnum outnum, int vol_mv)
{
#if 0
	struct SM5007_regulator *ri = 0;

	switch(outnum) {
		case REGULATOR_CORE:
			ri = &SM5007_regulator[0];
			if ((vol_mv < 1000) || (vol_mv >1300)) {
				return -EINVAL;
			}
			break;
		case REGULATOR_MEM:
			ri = &SM5007_regulator[2];
			break;
		case REGULATOR_IO:
			ri = &SM5007_regulator[4];
			break;
		default:
			return -EINVAL;
	}

	if ((vol_mv < 600) || (vol_mv > 3500)) {
		return -EINVAL;
	}
	return __SM5007_set_voltage(SM5007_PMU_ADDR, ri, vol_mv * 1000, vol_mv * 1000);
#endif
}
#endif
