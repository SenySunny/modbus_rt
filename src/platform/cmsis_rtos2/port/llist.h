/**
 * @file llist.h
 * @brief
 * @author haoyue (yjxkxk@qq.com)
 * @version V 0.0.1
 * @date 2023-05-23 23:14:06 +0800
 *
 * @copyright Copyright (c) 2023  haoyue
 *
 * @par History:
 *
 * 1. First create
 *    @date 2023-05-23 23:14:06 +0800     @author  haoyue (yjxkxk@qq.com)
 *    description: Create file for first time.
 */
/*! @note As a user, do not modify the content of this file!!!   */
/*! @note do not move this pre-processor statement to other places  */
#ifndef __LLIST_H__
#define __LLIST_H__
/*============================ INCLUDES ======================================*/
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*============================ MACROS ========================================*/
#define LL_INIT(N) ((N)->next = (N)->prev = (N))

#define LL_HEAD(H) llhead_t H = { &H, &H }

#define LL_ENTRY(P,T,N) ((T *)((char *)(P) - offsetof(T, N)))


/*============================ MACROFIED FUNCTIONS ===========================*/
#define LL_ADD(H, N)       \
 do {        \
  ((H)->next)->prev = (N);    \
  (N)->next = ((H)->next);    \
  (N)->prev = (H);     \
  (H)->next = (N);     \
 } while (0)

#define LL_TAIL(H, N)       \
 do {        \
  ((H)->prev)->next = (N);    \
  (N)->prev = ((H)->prev);    \
  (N)->next = (H);     \
  (H)->prev = (N);     \
 } while (0)

#define LL_DEL(N)       \
 do {        \
  ((N)->next)->prev = ((N)->prev);   \
  ((N)->prev)->next = ((N)->next);   \
  LL_INIT(N);      \
 } while (0)

#define LL_EMPTY(N) ((N)->next == (N))

#define LL_FOREACH(H,N) for (N = (H)->next; N != (H); N = (N)->next)

#define LL_FOREACH_SAFE(H,N,T)      \
 for (N = (H)->next, T = (N)->next; N != (H);   \
   N = (T), T = (N)->next)

/*============================ TYPES =========================================*/
typedef struct llhead_t     llhead_t;
struct llhead_t {
 llhead_t *prev;
 llhead_t *next;
};
/*============================ GLOBAL VARIABLES ==============================*/


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LLIST_H__ */
/********************** (C) haoyue *****end of llist.h**************************/
