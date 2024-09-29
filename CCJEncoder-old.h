
template <int T>
class CCJEncoder
{
public:
    static CCJEncoder<T>* instance;

    CCJEncoder()  {}
    ~CCJEncoder() {}

    void init(int ENC_A, int ENC_B)
    {
        CCJEncoder<T>::instance = this;
      /* for my hardware
      #define ENC_A	10
      #define ENC_B	15
      #define ENC_SW	16
      
      */  

    mPinA = ENC_A;
    mPinB = ENC_B;

    gpio_init(ENC_A);
    gpio_set_dir(ENC_A,GPIO_IN);
	gpio_pull_up(ENC_A);

	gpio_init(ENC_B);
    gpio_set_dir(ENC_B,GPIO_IN);
	gpio_pull_up(ENC_B);

//change this to RP2040 SDK style.  https://forums.raspberrypi.com/viewtopic.php?t=314186
        // Attach interrupts to A and B pins
//       attachInterrupt(digitalPinToInterrupt(mPinA), callback, CHANGE);
//       attachInterrupt(digitalPinToInterrupt(mPinB), callback, CHANGE);
         uint32_t event_mask = GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE;
         gpio_set_irq_enabled_with_callback(ENC_A,event_mask, true, &callback);
         gpio_set_irq_enabled_with_callback(ENC_B,event_mask, true, &callback);
    }

    bool check()
    { 
        if (mUpdatePending)
        {
            processChange();
            return mValueChanged;
        }
        return false;
    }
    
    void setValue(int value)  { mValue = value; mValueChanged = true; }
    int  value()              { return mValue;  mValueChanged = false; }
    bool valueChanged()       { return mValueChanged; }

private:

    static void callback()
    {
        instance->mUpdatePending = true;
    }
    
    void processChange()
    {
        mValueChanged = false;

        mUpdatePending = false;
        static uint8_t lrmem = 3;
        static int     lrsum = 0;
        static int8_t  TRANS[] = { 0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0 };

        // Read value from pin A and pin B
      //  int8_t l = digitalRead(mPinA);
      //  int8_t r = digitalRead(mPinB);

        int8_t l = gpio_get(mPinA);
        int8_t r = gpio_get(mPinB);

        lrmem = ((lrmem & 0x03) << 2) | (l << 1) | r;
        
        lrsum += TRANS[lrmem];

        if (lrsum % 4 != 0)
        {
            return;
        }

        if (lrsum == 4)
        {
            mValueChanged = true;
            lrsum = 0;
            mValue++;
            return;
        }

        if (lrsum == -4)
        {
            mValueChanged = true;
            lrsum = 0;
            mValue--;
            return;
        }

        lrsum = 0;
    }

private:
    int   mValue;
    int   mPinA;
    int   mPinB;
    bool  mValueChanged;
    volatile bool mUpdatePending;
};
template <int T> CCJEncoder<T>* CCJEncoder<T>::instance;
