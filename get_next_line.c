/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/20 21:56:33 by abelov            #+#    #+#             */
/*   Updated: 2023/11/20 21:56:35 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

int	makebuf(t_fp *fp)
{
	void	*p;

	p = malloc(BUFFER_SIZE);
	if (!p)
	{
		fp->_flags |= UN_BUF;
		fp->ptr = fp->nbuf;
		fp->sbf._base = fp->ptr;
		fp->sbf._size = 1;
		fp->_flags |= FOUND_ERR;
		return (-1);
	}
	fp->_flags |= MBF;
	fp->ptr = (unsigned char *)p;
	fp->sbf._base = p;
	fp->sbf._size = BUFFER_SIZE;
	return (0);
}

int	refill(t_fp *fp)
{
	if (!fp->sbf._base && makebuf(fp))
		return (EOF);
	fp->ptr = fp->sbf._base;
	fp->_r = read(fp->_file, (char *)fp->ptr, fp->sbf._size);
	fp->_flags &= ~SMOD;
	if (fp->_r <= 0)
	{
		if (fp->_r == 0)
			fp->_flags |= FOUND_EOF;
		else
		{
			fp->_r = 0;
			if (fp->_flags & MBF)
			{
				free(fp->lbf._base);
				fp->lbf._base = NULL;
				fp->lbf._size = 0;
			}
			fp->_flags |= FOUND_ERR;
		}
		return (EOF);
	}
	return (0);
}

t_fp	*sfp(int fd)
{
	t_fp	*fp;

	fp = (t_fp *)malloc(sizeof(t_fp) * 1);
	if (!fp)
		return (NULL);
	fp->ptr = NULL;
	fp->_file = fd;
	fp->_flags = 1;
	fp->_r = 0;
	fp->sbf._base = NULL;
	fp->sbf._size = 0;
	fp->lbf._base = NULL;
	fp->lbf._size = 0;
	return (fp);
}

char	*ft_fgetln(t_fp *fp)
{
	size_t			len;
	size_t			offset;

	if (fp->_r <= 0 && refill(fp))
		return (NULL);
	offset = 0;
	len = fp->_r;
	if (get_str(fp, len, offset))
		return ((char *)fp->lbf._base);
	while (fp->_r > 0)
	{
		lbchange(fp, len + OPTIMISTIC, !DO_SHRINK);
		(void)ft_memcpy(fp->lbf._base + offset, fp->ptr, len - offset);
		offset = len;
		if (refill(fp) && (fp->_flags & FOUND_EOF)
			&& lbchange(fp, len + 1, DO_SHRINK))
			break ;
		if (fp->_flags & FOUND_ERR)
			return (NULL);
		if (get_str(fp, len, offset))
			return ((char *)fp->lbf._base);
		len += fp->_r;
	}
	fp->lbf._base[len] = '\0';
	return ((char *)fp->lbf._base);
}

char	*get_next_line(int fd)
{
	static t_fp	*fp = NULL;
	char		*buf;

	if (fd < 0 || fd > SHRT_MAX)
		return (NULL);
	if (!fp)
		fp = sfp(fd);
	fp->lbf._base = NULL;
	fp->lbf._size = 0;
	buf = ft_fgetln(fp);
	if (!buf)
	{
		if (fp->_flags & MBF)
		{
			free(fp->sbf._base);
			fp->sbf._base = NULL;
			fp->sbf._size = 0;
		}
		free(fp->lbf._base);
		fp->lbf._base = NULL;
		fp->lbf._size = 0;
		free(fp);
		fp = NULL;
	}
	return (buf);
}
