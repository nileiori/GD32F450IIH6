#include "string.h"
#include <drv_can.h>
#include <drv_gpio.h>



#define BX_CAN_FMRNUMBER 28
#define BX_CAN2_FMRSTART 14

#define BX_CAN_MAX_FILTERS (BX_CAN_FMRNUMBER * 4)
#define BX_CAN_MAX_FILTER_MASKS BX_CAN_MAX_FILTERS
#define BX_CAN_FILTER_MAX_ARRAY_SIZE ((BX_CAN_MAX_FILTERS + 32 - 1) / 32)

struct gd_bxcanfiltermap
{
    uint32_t id32mask_cnt;
    uint32_t id32bit_cnt;
    uint32_t id16mask_cnt;
    uint32_t id16bit_cnt;
};
struct gd_bxcanfilter_masks
{
    uint32_t id32maskm[BX_CAN_FILTER_MAX_ARRAY_SIZE];
    uint32_t id32bitm[BX_CAN_FILTER_MAX_ARRAY_SIZE];
    uint32_t id16maskm[BX_CAN_FILTER_MAX_ARRAY_SIZE];
    uint32_t id16bitm[BX_CAN_FILTER_MAX_ARRAY_SIZE];
    uint32_t id32maskshift[2];
    uint32_t id32bitshift[2];
    uint32_t id16maskshift[2];
    uint32_t id16bitshift[2];
};

struct gd_bxcan
{
    uint32_t can_periph;
    void *mfrbase;
    IRQn_Type sndirq;
    IRQn_Type rcvirq0;
    IRQn_Type rcvirq1;
    IRQn_Type errirq;
    struct gd_bxcanfilter_masks filtermask;
    uint32_t alocmask[BX_CAN_FILTER_MAX_ARRAY_SIZE];
    uint32_t filtercnt;
    const uint32_t fifo1filteroff;
    const struct gd_bxcanfiltermap filtermap[2];
    uint32_t	 canpwrpin;
};
struct gd_baud_rate_tab
{
    uint32_t baud_rate;
    uint32_t confdata;
};


#define BS1SHIFT 16
#define BS2SHIFT 20
#define RRESCLSHIFT 0
#define SJWSHIFT 24
#define BS1MASK ( (0x0F) << BS1SHIFT )
#define BS2MASK ( (0x07) << BS2SHIFT )
#define RRESCLMASK ( 0x3FF << RRESCLSHIFT )
#define SJWMASK ( 0x3 << SJWSHIFT )

#define MK_BKCAN_BAUD(SJW,BS1,BS2,PRES) \
    ((SJW << SJWSHIFT) | (BS1 << BS1SHIFT) | (BS2 << BS2SHIFT) | (PRES << RRESCLSHIFT))

static const struct gd_baud_rate_tab bxcan_baud_rate_tab[] =
{
    // PCLK1 = HCLK / 4 = 50 M
    {1000UL * 1000, 	MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 5)},
    {1000UL * 500,	MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 10)},
    {1000UL * 250,	MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 20)},
    {1000UL * 125,	MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 40)},
    {1000UL * 100,	MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 50)},
    {1000UL * 50,		MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 100)},
    {1000UL * 20,		MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 250)},
    {1000UL * 10,		MK_BKCAN_BAUD(CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ,  CAN_BT_BS2_2TQ, 500)}
};

#define BAUD_DATA(TYPE,NO) \
    ((bxcan_baud_rate_tab[NO].confdata & TYPE##MASK) >> TYPE##SHIFT)

static uint32_t bxcan_get_baud_index(uint32_t baud)
{
    uint32_t len, index, default_index;

    len = sizeof(bxcan_baud_rate_tab) / sizeof(bxcan_baud_rate_tab[0]);
    default_index = len;

    for(index = 0; index < len; index++)
    {
        if(bxcan_baud_rate_tab[index].baud_rate == baud)
            return index;

        if(bxcan_baud_rate_tab[index].baud_rate == 1000UL * 250)
            default_index = index;
    }

    if(default_index != len)
        return default_index;

    return 0;
}


static void bxcan_init(uint32_t can_periph, uint32_t baud, uint32_t mode)
{
    can_parameter_struct can_parameter;

    uint32_t baud_index = bxcan_get_baud_index(baud);

    /* initialize CAN register */
    can_deinit(can_periph);

    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = ENABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;

    switch (mode)
    {
        case GD_CAN_MODE_NORMAL:
            can_parameter.working_mode = CAN_NORMAL_MODE;
            break;

        case GD_CAN_MODE_LISEN:
            can_parameter.working_mode = CAN_SILENT_MODE;
            break;

        case GD_CAN_MODE_LOOPBACK:
            can_parameter.working_mode = CAN_LOOPBACK_MODE;
            break;

        case GD_CAN_MODE_LOOPBACKANLISEN:
            can_parameter.working_mode = CAN_SILENT_LOOPBACK_MODE;
            break;
    }

    can_parameter.resync_jump_width = BAUD_DATA(SJW, baud_index);
    can_parameter.time_segment_1 = BAUD_DATA(BS1, baud_index);
    can_parameter.time_segment_2 = BAUD_DATA(BS2, baud_index);
    can_parameter.prescaler = BAUD_DATA(RRESCL, baud_index);

    /* initialize CAN */
    can_init(can_periph, &can_parameter);
}

#ifdef USING_BXCAN0
static void bxcan0_hw_init(void)
{
    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOA);

    /* configure CAN1 GPIO */
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_11);

    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_12);

    bxcan_init(CAN0, CAN500kBaud, GD_CAN_MODE_NORMAL);
    /* configure CAN0 NVIC */
    //nvic_irq_enable(CAN0_RX0_IRQn,0,0);

    /* enable CAN receive FIFO0 not empty interrupt */
    //can_interrupt_enable(CAN0, CAN_INT_RFNE0);
}
#endif
#ifdef USING_BXCAN1
static void bxcan1_hw_init(void)
{
    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN1);
    rcu_periph_clock_enable(RCU_GPIOB);

    /* configure CAN2 GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_12);

    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_13);

    bxcan_init(CAN1, CAN500kBaud, GD_CAN_MODE_NORMAL);

    /* configure CAN1 NVIC */
    //nvic_irq_enable(CAN1_RX0_IRQn,1,0);

    /* enable CAN receive FIFO0 not empty interrupt */
    //can_interrupt_enable(CAN1, CAN_INT_RFNE0);
}

#endif

void bxcan_set_baud_rate(uint32_t can_periph, uint32_t baud)
{
    can_parameter_struct can_parameter;

    uint32_t baud_index = bxcan_get_baud_index(baud);

    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = ENABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;

    can_parameter.resync_jump_width = BAUD_DATA(SJW, baud_index);
    can_parameter.time_segment_1 = BAUD_DATA(BS1, baud_index);
    can_parameter.time_segment_2 = BAUD_DATA(BS2, baud_index);
    can_parameter.prescaler = BAUD_DATA(RRESCL, baud_index);

    /* initialize CAN */
    can_init(can_periph, &can_parameter);
}

void bxcan_pwr_configure(uint32_t pwrpin, uint32_t pinVal)
{
    gd32_pin_mode(pwrpin, PIN_MODE_OUTPUT);
    gd32_pin_write(pwrpin, pinVal);

}
void bxcan_format_data(can_receive_message_struct rxMsg, TCanData* data)
{
    data->len = rxMsg.rx_dlen;
    memcpy(data->data, rxMsg.rx_data, 8);

    if (rxMsg.rx_ff == CAN_FF_STANDARD)
    {
        data->id = rxMsg.rx_sfid;
        data->idType = IDType_Standard;
    }
    else
    {
        data->id = rxMsg.rx_efid;
        data->idType = IDType_Extend;
    }

    if (rxMsg.rx_ft == CAN_FT_DATA)
    {
        data->frameType = FrameType_Data;
    }
    else
    {
        data->frameType = FrameType_Remote;
    }
}

//static int sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno)

//static int recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno)

void bxcan_format_filter(TFilter* filter, uint8_t num, ECanFrameType frameType, ECanIdType idType, uint32_t id)
{
    filter->filterNo = num;
    filter->idType = idType;
    filter->frameType = frameType;
    filter->filterID = id;
}

void bxcan_filter_config(TCanConfig config)
{
    uint32_t can_periph;
    can_filter_parameter_struct can_filter;
    int i = 0;

    /* CAN filter init */
    if (config.ch == CAN_1)
    {
        can_periph = CAN0;
    }
    else
    {
        can_periph = CAN1;
    }

    if (config.filterUsedNum > 0)
    {
        can_filter.filter_mode = CAN_FILTERMODE_MASK;
        can_filter.filter_bits = CAN_FILTERBITS_32BIT;
        can_filter.filter_fifo_number = CAN_FIFO0;
        can_filter.filter_enable = ENABLE;

        for (i = 0; i < config.filterUsedNum; i++)
        {
            if (can_periph == CAN0)
            {
                can_filter.filter_number = config.filter[i].filterNo;
            }
            else
            {
                can_filter.filter_number = 14 + config.filter[i].filterNo;
            }

            can_filter.filter_list_high = 0x0000;
            can_filter.filter_list_low = 0x0000;
            can_filter.filter_mask_high = 0x0000;
            can_filter.filter_mask_low = 0x0000;

            if (config.filter[i].idType == IDType_Standard)
            {
                can_filter.filter_list_high = (config.filter[i].filterID << 5) & 0xFFFF;
                can_filter.filter_list_low = 0x0000;
                can_filter.filter_list_low &= ~(1 << 2);
                can_filter.filter_mask_high = 0xFFFF;
                can_filter.filter_mask_low = 0xFFFF;
            }
            else
            {
                can_filter.filter_list_high = (config.filter[i].filterID >> 13) & 0xFFFF;
                can_filter.filter_list_low = (config.filter[i].filterID << 3) & 0xFFFF;
                can_filter.filter_list_low |= (1 << 2);
                can_filter.filter_mask_high = 0xFFFF;
                can_filter.filter_mask_low = 0xFFFF;
            }

            if (config.filter[i].frameType == FrameType_Remote)
            {
                can_filter.filter_list_low |= (1 << 1);
            }
            else
            {
                can_filter.filter_list_low &= ~(1 << 1);
            }

        }
    }
    else
    {
        if (can_periph == CAN0)
        {
            can_filter.filter_number = 0;
        }
        else
        {
            can_filter.filter_number = 14;
        }

        can_filter.filter_mode = CAN_FILTERMODE_MASK;
        can_filter.filter_bits = CAN_FILTERBITS_32BIT;
        can_filter.filter_list_high = 0x0000;
        can_filter.filter_list_low = 0x0000;
        can_filter.filter_mask_high = 0x0000;
        can_filter.filter_mask_low = 0x0000;
        can_filter.filter_fifo_number = CAN_FIFO0;
        can_filter.filter_enable = ENABLE;

    }

    can_filter_init(&can_filter);

    if (config.bUseInterrupt)
    {
        if (can_periph == CAN0)
        {
            nvic_irq_enable(CAN0_RX0_IRQn, 0, 0);

            can_interrupt_enable(CAN0, CAN_INT_RFNE0);
        }
        else
        {
            nvic_irq_enable(CAN1_RX0_IRQn, 1, 0);

            can_interrupt_enable(CAN1, CAN_INT_RFNE0);
        }
    }
}
TCanConfig can0_config =
{
    .ch = CAN_1,
    .filterUsedNum = 2,
    .bUseInterrupt = true
};

TCanConfig can1_config =
{
    .ch = CAN_2,
    .filterUsedNum = 3,
    .bUseInterrupt = true
};

//标准CAN ID与扩展CAN ID混合过滤
//定义一组标准CAN ID
uint32_t StdIdArray[] =
{
    0x711,
    0x712,
    0x713,
    0x714,
    0x715,
};
//定义另外一组扩展CAN ID
uint32_t ExtIdArray[] =
{
    0x1900fAB1,
    0x1900fAB2,
    0x1900fAB3,
    0x1900fAB4,
    0x1900fAB5,
    0x1900fAB6
};

void CANFilterConfig_Scale32_IdMask_StandardId_ExtendId_Mix(void)
{
    can_filter_parameter_struct can_filter;
    uint32_t      mask, num, tmp, i, standard_mask, extend_mask, mix_mask;

    can_filter.filter_number = 4;               //使用过滤器4
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = ((ExtIdArray[0] << 3) >> 16) & 0xffff; //使用第一个扩展CAN  ID作为验证码
    can_filter.filter_list_low = ((ExtIdArray[0] << 3) & 0xffff);

    standard_mask = 0x7ff;    //下面是计算屏蔽码
    num = sizeof(StdIdArray) / sizeof(StdIdArray[0]);

    for(i = 0; i < num; i++)       //首先计算出所有标准CAN ID的屏蔽码
    {
        tmp = StdIdArray[i] ^ (~StdIdArray[0]);
        standard_mask &= tmp;
    }

    extend_mask = 0x1fffffff;
    num = sizeof(ExtIdArray) / sizeof(ExtIdArray[0]);

    for(i = 0; i < num; i++)       //接着计算出所有扩展CAN ID的屏蔽码
    {
        tmp = ExtIdArray[i] ^ (~ExtIdArray[0]);
        extend_mask &= tmp;
    }

    mix_mask = (StdIdArray[0] << 18) ^ (~ExtIdArray[0]); //再计算标准CAN ID与扩展CAN ID混合的屏蔽码
    mask = (standard_mask << 18)& extend_mask & mix_mask; //最后计算最终的屏蔽码
    mask <<= 3;                         //对齐寄存器

    can_filter.filter_mask_high = (mask >> 16) & 0xffff;
    can_filter.filter_mask_low = (mask & 0xffff);
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;

    can_filter_init(&can_filter);
}

void gd32_bxcan_init(void)
{

    bxcan0_hw_init();
    bxcan1_hw_init();
    // Initialize CAN1 Filter
    bxcan_format_filter(&can0_config.filter[0], 0, FrameType_Data, IDType_Extend, 0x12);
    bxcan_format_filter(&can0_config.filter[1], 1, FrameType_Data, IDType_Standard, 0x18);
    bxcan_filter_config(can0_config);
    // Initialize CAN2 Filter
    bxcan_format_filter(&can1_config.filter[0], 0, FrameType_Data, IDType_Extend, 0x12);
    bxcan_format_filter(&can1_config.filter[1], 1, FrameType_Data, IDType_Standard, 0x18);
    bxcan_format_filter(&can1_config.filter[2], 2, FrameType_Data, IDType_Extend, 0x1ABB);
    bxcan_filter_config(can1_config);
}

bool bxcan_recv_data(ECanChannel ch, TCanData* data)
{
    can_receive_message_struct msg;
    bool retVal = false;

    if (ch == CAN_1)
    {
        if (can_flag_get(CAN0, CAN_FLAG_RFF0) == SET)
        {
            can_message_receive(CAN0, CAN_FIFO0, &msg);
            bxcan_format_data(msg, data);
            retVal = true;
        }
    }
    else
    {
        if (can_flag_get(CAN1, CAN_FLAG_RFF0) == SET)
        {
            can_message_receive(CAN1, CAN_FIFO0, &msg);
            bxcan_format_data(msg, data);
            retVal = true;
        }
    }

    return retVal;
}

bool bxcan_send_data(ECanChannel ch, TCanData data)
{
    can_trasnmit_message_struct tx_msg;

    if (data.idType == IDType_Standard)
    {
        tx_msg.tx_ff = CAN_FF_STANDARD;
        tx_msg.tx_sfid = data.id;
    }
    else
    {
        tx_msg.tx_ff = CAN_FF_EXTENDED;
        tx_msg.tx_efid = data.id;
    }

    if (data.frameType == FrameType_Data)
    {
        tx_msg.tx_ft = CAN_FT_DATA;
    }
    else
    {
        tx_msg.tx_ft = CAN_FT_REMOTE;
    }

    tx_msg.tx_dlen = data.len;
    memcpy(tx_msg.tx_data, data.data, 8);

    if (ch == CAN_1)
    {
        can_message_transmit(CAN0, &tx_msg);
        can_interrupt_enable(CAN0, CAN_INT_TME);
    }
    else
    {
        can_message_transmit(CAN1, &tx_msg);
        can_interrupt_enable(CAN1, CAN_INT_TME);
    }

    return true;
}

void CAN0_RX0_IRQHandler(void)
{
    can_receive_message_struct msg;

    if(can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFF0) == SET)
    {
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
        can_message_receive(CAN0, CAN_FIFO0, &msg);

    }
}

void CAN0_TX_IRQHandler(void)
{

}

void CAN1_RX0_IRQHandler(void)
{
    can_receive_message_struct msg;

    if(can_interrupt_flag_get(CAN1, CAN_INT_FLAG_RFF0) == SET)
    {
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
        can_message_receive(CAN1, CAN_FIFO0, &msg);
    }

}

