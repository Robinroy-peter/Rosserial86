/***********************  Recursive IRQ Disable/Enable  ***********************/
#if defined(DMP_DOS_DJGPP)
    #define DISABLE_INT()       __asm__ __volatile__("cli" : : : "memory")
    #define ENABLE_INT()        __asm__ __volatile__("sti" : : : "memory")
#elif defined(DMP_DOS_WATCOM)
    #define DISABLE_INT()       _disable()
    #define ENABLE_INT()        _enable()
#elif defined(DMP_DOS_BC)
    #define DISABLE_INT()       disable()
    #define ENABLE_INT()        enable()
#else
    #define DISABLE_INT()
    #define ENABLE_INT()
#endif

// well ... bad design:(
#define _io_SetCSCNT(val) do { \
                              ATOMIC_INT_SET(&irq_cscnt, val); \
                          } while(0)

#define _io_GetCSCNT()    ATOMIC_INT_GET(&irq_cscnt)

#define _io_DisableINT()  do { \
                              if (ATOMIC_INT_TEST(&irq_cscnt, 0)) { DISABLE_INT(); } \
                              ATOMIC_INT_INC(&irq_cscnt); \
                              ATOMIC_MEM_BARRIER(); \
                          } while(0)

#define _io_RestoreINT()  do { \
                              ATOMIC_MEM_BARRIER(); \
                              if (!ATOMIC_INT_TEST(&irq_cscnt, 0)) { \
                                  ATOMIC_INT_DEC(&irq_cscnt); \
                                  if (ATOMIC_INT_TEST(&irq_cscnt, 0)) { ENABLE_INT(); } \
                              } \
                          } while(0)

#define _io_EnableINT()   do { \
                              ATOMIC_MEM_BARRIER(); \
                              if (!ATOMIC_INT_TEST(&irq_cscnt, 0)) { \
                                  ATOMIC_INT_SET(&irq_cscnt, 0); \
                                  ENABLE_INT(); \
                              } \
                          } while(0)


/*
    DMPAPI(void) io_SetCSCNT(int val) {
        ATOMIC_INT_SET(&irq_cscnt, val);
    } DPMI_END_OF_LOCKED_FUNC(io_SetCSCNT)

    DMPAPI(int) io_GetCSCNT(void) {
        return ATOMIC_INT_GET(&irq_cscnt);
    } DPMI_END_OF_LOCKED_FUNC(io_GetCSCNT)

    DMPAPI(void) io_DisableINT(void) {
        if (ATOMIC_INT_TEST(&irq_cscnt, 0))
        {
            DISABLE_INT();
        }
        // getting interrupts disabled, it is now safe to update irq_cscnt
        ATOMIC_INT_INC(&irq_cscnt);
    } DPMI_END_OF_LOCKED_FUNC(io_DisableINT)

    DMPAPI(void) io_RestoreINT(void) {
        if (ATOMIC_INT_TEST(&irq_cscnt, 0)) return;
        // having interrupts disabled, it is safe to update irq_cscnt
        ATOMIC_INT_DEC(&irq_cscnt);
        if (ATOMIC_INT_TEST(&irq_cscnt, 0))
        {
            ENABLE_INT();
        }
    } DPMI_END_OF_LOCKED_FUNC(io_RestoreINT)

    DMPAPI(void) io_EnableINT(void) {
        if (ATOMIC_INT_TEST(&irq_cscnt, 0)) return;
        // having interrupts disabled, it is safe to update irq_cscnt
        ATOMIC_INT_SET(&irq_cscnt, 0);
        ENABLE_INT();
    } DPMI_END_OF_LOCKED_FUNC(io_EnableINT)
*/

/*-------------------  end. Recursive IRQ Disable/Enable  --------------------*/

