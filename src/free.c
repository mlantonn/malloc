/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlantonn <mlantonn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 17:36:00 by mlantonn          #+#    #+#             */
/*   Updated: 2019/10/17 16:59:43 by mlantonn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/mman.h>
#include "mymalloc.h"

t_zone	*match_ptr(t_zone *zone, void *ptr, int *i)
{
	t_zone	*tmp;
	size_t	coeff;

	if (zone == NULL)
		return (NULL);
	tmp = zone;
	coeff = zone->kind == TINY ? g_data.tiny_coeff : g_data.small_coeff;
	while (1)
	{
		if (tmp->areas_left != zone->limit)
		{
			*i = -1;
			while (++(*i) < zone->limit)
				if (tmp->allocated[*i] && ((tmp->ptr + (coeff * *i)) == ptr))
					return (tmp);
		}
		tmp = tmp->next;
		if (tmp->is_root)
			break ;
	}
	return (NULL);
}

void	match_zone_ptr(t_zone **zone, void *ptr, int *i)
{
	*i = 0;
	*zone = match_ptr(g_data.tiny, ptr, i);
	if (!(*zone))
		*zone = match_ptr(g_data.small, ptr, i);
	if (!(*zone))
		*zone = match_ptr(g_data.large, ptr, i);
}

void	free_area(t_zone *zone, t_zone **g_data_ptr, int i)
{
	zone->allocated[i] = 0;
	zone->areas_left++;
	if (zone->areas_left == zone->limit)
	{
		munmap(zone->ptr, zone->size);
		if (zone == zone->next)
			*g_data_ptr = NULL;
		else
		{
			if (zone->is_root)
			{
				zone->next->is_root = 1;
				*g_data_ptr = zone->next;
			}
			zone->prev->next = zone->next;
			zone->next->prev = zone->prev;
		}
		munmap(zone, sizeof(t_zone));
	}
}

void	free(void *ptr)
{
	t_zone	*zone;
	int		i;

	if (!ptr)
		return ;
	pthread_mutex_lock(&g_mtx.free);
	if ((zone = match_ptr(g_data.tiny, ptr, &i)))
		free_area(zone, &g_data.tiny, i);
	else if ((zone = match_ptr(g_data.small, ptr, &i)))
		free_area(zone, &g_data.small, i);
	else if ((zone = match_ptr(g_data.large, ptr, &i)))
		free_area(zone, &g_data.large, i);
	pthread_mutex_unlock(&g_mtx.free);
}
