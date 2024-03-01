/**
 * @file modbus_rt_platform_memory.c
 * @brief
 * @author haoyue (yjxkxk@qq.com)
 * @version V 0.0.1
 * @date 2024-01-26 11:38:48 +0800
 *
 * @copyright Copyright (c) 2024  haoyue
 *
 * @par History:
 *
 * 1. First create
 *    @date 2024-01-26 11:38:48 +0800     @author  haoyue (yjxkxk@qq.com)
 *    description: Create file for first time.
 */
/*! @note As a user, do not modify the content of this file!!!   */
/*! @note do not move this pre-processor statement to other places  */
/*============================ INCLUDES ======================================*/
#include "./modbus_rt_platform_memory.h"
#include "./port/llist.h"
/*============================ MACROS ========================================*/

/*============================ MACROFIED FUNCTIONS ===========================*/

/*============================ TYPES =========================================*/
typedef struct modbus_rt_mem_link_t  modbus_rt_mem_link_t;
struct modbus_rt_mem_link_t {
    llhead_t                tLink;
    osMemoryPoolId_t        ptLinkPool;
    osMemoryPoolId_t        ptMemPool;
    void *                  pMem;
};
/*============================ PROTOTYPES ====================================*/

/*============================ LOCAL VARIABLES ===============================*/
static LL_HEAD(s_tModbusRtMemList);
/*============================ GLOBAL VARIABLES ==============================*/

/*============================ IMPLEMENTATION ================================*/

/********************** (C) haoyue *****end of modbus_rt_platform_memory.c**************************/
void *modbus_rt_malloc(size_t size) {
    modbus_rt_mem_link_t *ptLink = NULL;
    void *pMem = NULL;
    osMemoryPoolId_t ptMemPool = NULL, ptLinkPool = NULL;

    ptLinkPool = osMemoryPoolNew(1u, sizeof(modbus_rt_mem_link_t), NULL);
    if(NULL == ptLinkPool) {
        return NULL;
    }
    ptLink = osMemoryPoolAlloc(ptLinkPool, 5u);
    if(NULL == ptLink) {
        return NULL;
    }
    memset(ptLink, 0, sizeof(modbus_rt_mem_link_t));
    LL_INIT(&ptLink->tLink);
    ptLink->ptLinkPool = ptLinkPool;
    ptMemPool = osMemoryPoolNew(1u, size, NULL);
    if(NULL == ptMemPool) {
        osMemoryPoolFree(ptLinkPool, ptLink);
        osMemoryPoolDelete(ptLinkPool);
        return NULL;
    }
    ptLink->ptMemPool = ptMemPool;
    pMem = osMemoryPoolAlloc(ptMemPool, 5u);
    if(NULL == pMem) {
        osMemoryPoolFree(ptLinkPool, ptLink);
        osMemoryPoolDelete(ptLinkPool);
        osMemoryPoolDelete(ptMemPool);
        return NULL;
    }
    memset(pMem, 0, size);
    ptLink->pMem = pMem;
    LL_TAIL(&s_tModbusRtMemList, &ptLink->tLink);

    return pMem;
}

void *modbus_rt_calloc(size_t num, size_t size) {
    modbus_rt_mem_link_t *ptLink = NULL;
    void *pMem = NULL;
    osMemoryPoolId_t ptMemPool = NULL, ptLinkPool = NULL;

    ptLinkPool = osMemoryPoolNew(1u, sizeof(modbus_rt_mem_link_t), NULL);
    if(NULL == ptLinkPool) {
        return NULL;
    }
    ptLink = osMemoryPoolAlloc(ptLinkPool, 5u);
    if(NULL == ptLink) {
        return NULL;
    }
    LL_INIT(&ptLink->tLink);
    ptLink->ptLinkPool = ptLinkPool;
    ptMemPool = osMemoryPoolNew(1u, num * size, NULL);
    if(NULL == ptMemPool) {
        osMemoryPoolFree(ptLinkPool, ptLink);
        osMemoryPoolDelete(ptLinkPool);
        return NULL;
    }
    ptLink->ptMemPool = ptMemPool;
    pMem = osMemoryPoolAlloc(ptMemPool, 5u);
    if(NULL == pMem) {
        osMemoryPoolFree(ptLinkPool, ptLink);
        osMemoryPoolDelete(ptLinkPool);
        osMemoryPoolDelete(ptMemPool);
        return NULL;
    }
    ptLink->pMem = pMem;
    LL_TAIL(&s_tModbusRtMemList, &ptLink->tLink);

    return pMem;
}

void modbus_rt_free(void *ptr) {
    llhead_t *ptNode = NULL, *ptTmp = NULL;
    modbus_rt_mem_link_t *ptLink = NULL;

    if(NULL == ptr) {
        return;
    }

    LL_FOREACH_SAFE(&s_tModbusRtMemList, ptNode, ptTmp) {
        ptLink = LL_ENTRY(ptNode, modbus_rt_mem_link_t, tLink);
        if(ptr == ptLink->pMem) {
            osMemoryPoolFree(ptLink->ptMemPool, ptLink->pMem);
            osMemoryPoolDelete(ptLink->ptMemPool);
            LL_DEL(&ptLink->tLink);
            osMemoryPoolFree(ptLink->ptLinkPool, ptLink);
            osMemoryPoolDelete(ptLink->ptLinkPool);
            return;
        }
    }
}
