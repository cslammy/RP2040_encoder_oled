//////////////////////////////////////////////////////////////////////
//
// Encoder class
// Christer Janson, June 2024 - www.chrutil.com
// Class for handling multiple rotary encoders with either interrupt or polling
// No hardware debouncing necessary
// Algorithm lifted from https://www.pinteric.com/rotary.html
//
// Interrupt based encoder class
// Each instance needs its own specialized version that cannot be shared.
// So declare each encoder with its own type: CCJISREncoder<1>, CCJISREncoder<2>, CCJISREncoder<3>, etc.
// CCJISREncoder<1> encoder1;
// CCJISREncoder<2> encoder2;
//
// Polling based encoder class.
// CCJPollEncoder encoder1;
// CCJPollEncoder encoder2;
//
// Usage: [Polling or ISR version has identical usage]
// void setup() 
// {
//     // Initialize encoder at pin 2 and 3
//     encoder1.init(2, 3);
// }
// void loop() 
// {
//     if (encoder1.check())
//     {
//         Serial.println(encoder1.value());
//     }
// }

#include <pico/stdlib.h>
// Base class for interrupt or polling encoder classes
class CCJBaseEncoder
{
public:
    // Initialize encoder with pin A and pin B
    void init(int pin_a, int pin_b)
    {
    gpio_init(pin_a);
    gpio_set_dir(pin_a,GPIO_IN);
	gpio_pull_up(pin_a); 

	gpio_init(pin_b);
    gpio_set_dir(pin_b,GPIO_IN);
    gpio_pull_up(pin_b);
	 
        
        
        mPinA = pin_a;
        mPinB = pin_b;



        initImp();
    }

    // Check if encoder has a new value.
    // Call from main loop since this does polling or value resolution
    virtual bool check() = 0;

    // Initialize encoder value 
    void setValue(int value)  { mValue = value; mValueChanged = true; }

    // Get encoder value - will reset the valuechanged flag
    int  value()              { return mValue;  mValueChanged = false; }

    // Check if encoder has been modified since last time you got its value
    bool valueChanged()       { return mValueChanged; }

protected:
    virtual void initImp() = 0;
    virtual void clearDirty() = 0;


    // The guts of this function was lifted from MP Electronics Devices at:
    // https://www.pinteric.com/rotary.html
    // I recommend visiting if you are interested in reading about how this actually works 
    void processChange()
    {
        mValueChanged = false;

        clearDirty();
        static int8_t  TRANS[] = { 0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0 };
  
        // Read value from pin A and pin B
        int8_t a = gpio_get(mPinA);
        int8_t b = gpio_get(mPinB);

        // Construct a four bit value with previous state and current state
        mABmem = ((mABmem & 0x03) << 2) | (a << 1) | b;
        
        mABsum += TRANS[mABmem];

        if (mABsum % 4 != 0)
        {
            return;
        }

        if (mABsum == 4)
        {
            mValueChanged = true;
            mABsum = 0;
            mValue++;
            return;
        }

        if (mABsum == -4)
        {
            mValueChanged = true;
            mABsum = 0;
            mValue--;
            return;
        }

        mABsum = 0;
    }
    
    int       mValue = 0;
    int       mPinA;
    int       mPinB;
    bool      mValueChanged;    
    uint8_t   mABmem = 3;
    int       mABsum = 0;    
};

template <int T>
class CCJISREncoder : public CCJBaseEncoder
{
public:
    static CCJISREncoder<T>* instance;

    CCJISREncoder()  {}
    ~CCJISREncoder() {}

    static void callback(uint, uint32_t)
    {
    instance ->mUpdatePending = true;  

    }
    
    void initImp() override
    {
        CCJISREncoder<T>::instance = this;
    
        // Attach interrupts to A and B pins
       // attachInterrupt(digitalPinToInterrupt(mPinA), callback, CHANGE);
       // attachInterrupt(digitalPinToInterrupt(mPinB), callback, CHANGE);
       gpio_set_irq_enabled_with_callback(mPinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &callback);
       gpio_set_irq_enabled_with_callback(mPinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &callback);
    }

    bool check() override
    { 
        if (mUpdatePending)
        {
            processChange();
            return mValueChanged;
        }
        return false;
    }
    
private:

    void clearDirty() override
    {
        mUpdatePending = false;
    }
    
    static void callback()
    {
        instance->mUpdatePending = true;
    }
    
private:
    volatile bool mUpdatePending;
};
template <int T> CCJISREncoder<T>* CCJISREncoder<T>::instance;

class CCJPollEncoder : public CCJBaseEncoder
{
public:

    CCJPollEncoder()  {}
    ~CCJPollEncoder() {}

    void initImp() override
    {
    }

    bool check() override
    {
        if (poll())
        {
            processChange();
            return mValueChanged;
        }
        return false;
    }
    
private:

    void clearDirty() override
    {
    }

    bool  poll()
    {
        int8_t a = gpio_get(mPinA);
        int8_t b = gpio_get(mPinB);
        //printf("a is: %d\n",a);
        //printf("b is: %d\n",b);
        if (a != ((mABmem & 0x02) >> 1) || (b != (mABmem & 0x01)))
            return true;

        return false;
    }
};
