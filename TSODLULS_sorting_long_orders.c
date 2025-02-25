/*
This file is part of TSODLULS library.

TSODLULS is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TSODLULS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with TSODLULS.  If not, see <http://www.gnu.org/licenses/>.

©Copyright 2018-2019 Laurent Lyaudet
*/

#include "TSODLULS.h"



//------------------------------------------------------------------------------------
//External functions
//------------------------------------------------------------------------------------
/**
 * Sorting functions for long nextified strings
 * The current state of the art sorting function for nextified strings.
 * Its implementation may change without warning.
 */
int TSODLULS_sort(t_TSODLULS_sort_element* arr_elements, size_t i_number_of_elements){
  return TSODLULS_sort_radix8_count_insertion(arr_elements, i_number_of_elements);
}//end function TSODLULS_sort()



/**
 * Sorting functions for long nextified strings
 * The current state of the art stable sorting function for nextified strings.
 * Its implementation may change without warning.
 */
int TSODLULS_sort_stable(t_TSODLULS_sort_element* arr_elements, size_t i_number_of_elements){
  return TSODLULS_sort_radix8_count_insertion(arr_elements, i_number_of_elements);
}//end function TSODLULS_sort_stable()



/**
 * Sorting functions for long nextified strings
 * A stable sorting algorithm for nextified strings based on radix sort with octets digits
 * and counting sort as a subroutine.
 */
int TSODLULS_sort_radix8_count(t_TSODLULS_sort_element* arr_elements, size_t i_number_of_elements){
  int i_number_of_distinct_bytes = 0;
  size_t arr_counts[256];
  size_t arr_offsets[256];
  uint8_t i_current_octet = 0;
  t_TSODLULS_sort_element* arr_elements_copy = NULL;
  t_TSODLULS_radix_instance current_instance;
  t_TSODLULS_radix_instance* arr_instances = NULL;
  size_t i_max_number_of_instances = 0;
  size_t i_current_instance = 0;
  void* p_for_realloc = NULL;
  size_t i_size_for_realloc = 0;

  if(i_number_of_elements < 2){
    return 0;//nothing to sort
  }

  current_instance.i_offset_first = 0;
  current_instance.i_offset_last = i_number_of_elements - 1;
  current_instance.i_depth = 0;
  current_instance.b_copy = 0;

  arr_elements_copy = calloc(i_number_of_elements, sizeof(t_TSODLULS_sort_element));
  if(arr_elements_copy == NULL){
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_instances = calloc(8, sizeof(t_TSODLULS_radix_instance));
  if(arr_instances == NULL){
    TSODLULS_free(arr_elements_copy);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  i_max_number_of_instances =  8;

  while(1){
    //we initialize the counters
    for(int i = 0; i < 256; ++i){
      arr_counts[i] = 0;
      arr_offsets[i] = 0;
    }
    //we count the bytes with a certain value
    if(current_instance.b_copy){
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements_copy[i].s_key[current_instance.i_depth];
        ++arr_counts[i_current_octet];
      }
    }
    else{
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements[i].s_key[current_instance.i_depth];
        ++arr_counts[i_current_octet];
      }
    }
    //we deduce the offsets
    i_number_of_distinct_bytes = 0;
    for(int i = 0; i < 255; ++i){
      arr_offsets[i+1] = arr_offsets[i] + arr_counts[i];
      if(arr_counts[i] > 0){
        ++i_number_of_distinct_bytes;
      }
    }
    if(arr_counts[255] > 0){
      ++i_number_of_distinct_bytes;
    }

    if(i_number_of_distinct_bytes == 1){
      ++current_instance.i_depth;
      if(current_instance.b_copy){
        //if we are done sorting this instance and all deeper subinstances
        if(arr_elements_copy[current_instance.i_offset_first].i_key_size <= current_instance.i_depth){
          memcpy(
              &(arr_elements[current_instance.i_offset_first]),
              &(arr_elements_copy[current_instance.i_offset_first]),
              (current_instance.i_offset_last - current_instance.i_offset_first + 1) * sizeof(t_TSODLULS_sort_element)
          );
          if(i_current_instance == 0){
            break;
          }
          current_instance = arr_instances[--i_current_instance];
        }
      }
      else{
        //if we are done sorting this instance and all deeper subinstances
        if(arr_elements[current_instance.i_offset_first].i_key_size <= current_instance.i_depth){
          if(i_current_instance == 0){
            break;
          }
          current_instance = arr_instances[--i_current_instance];
        }
      }
      continue;
    }

    if(current_instance.b_copy){
      //sorting
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements_copy[i].s_key[current_instance.i_depth];
        arr_elements[current_instance.i_offset_first + arr_offsets[i_current_octet]] = arr_elements_copy[i];
        ++arr_offsets[i_current_octet];
      }
      //instances creation
      for(int i = 0; i < 256; ++i){
        if(arr_counts[i] > 1//nothing to do for one element, result is in original array
          //for nextified strings this test can be done on only one element
          && arr_elements[current_instance.i_offset_first + arr_offsets[i] - 1].i_key_size
               > current_instance.i_depth + 1
        ){
          //we allocate more space for the new instance if necessary
          if(i_current_instance == i_max_number_of_instances - 1){
            i_size_for_realloc = i_max_number_of_instances * 2 * sizeof(t_TSODLULS_radix_instance);
            //if we cannot allocate more
            if(i_size_for_realloc <= i_max_number_of_instances * sizeof(t_TSODLULS_radix_instance)){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            p_for_realloc = realloc(arr_instances, i_size_for_realloc);
            if(p_for_realloc == NULL){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            arr_instances = (t_TSODLULS_radix_instance*)p_for_realloc;
            i_max_number_of_instances *= 2;
          }
          arr_instances[i_current_instance].i_offset_first = current_instance.i_offset_first
                                                           + arr_offsets[i] - arr_counts[i];
          arr_instances[i_current_instance].i_offset_last = current_instance.i_offset_first
                                                          + arr_offsets[i] - 1;
          arr_instances[i_current_instance].i_depth = current_instance.i_depth + 1;
          arr_instances[i_current_instance].b_copy = 0;
          ++i_current_instance;
        }
      }//end for(int i = 0; i < 256; ++i)
    }
    else{
      //sorting
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements[i].s_key[current_instance.i_depth];
        arr_elements_copy[current_instance.i_offset_first + arr_offsets[i_current_octet]] = arr_elements[i];
        ++arr_offsets[i_current_octet];
      }
      //instances creation
      for(int i = 0; i < 256; ++i){
        if(arr_counts[i] > 1
          //for nextified strings this test can be done on only one element
          && arr_elements_copy[current_instance.i_offset_first + arr_offsets[i] - 1].i_key_size
               > current_instance.i_depth + 1
        ){
          //we allocate more space for the new instance if necessary
          if(i_current_instance == i_max_number_of_instances - 1){
            i_size_for_realloc = i_max_number_of_instances * 2 * sizeof(t_TSODLULS_radix_instance);
            //if we cannot allocate more
            if(i_size_for_realloc <= i_max_number_of_instances * sizeof(t_TSODLULS_radix_instance)){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            p_for_realloc = realloc(arr_instances, i_size_for_realloc);
            if(p_for_realloc == NULL){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            arr_instances = (t_TSODLULS_radix_instance*)p_for_realloc;
            i_max_number_of_instances *= 2;
          }
          arr_instances[i_current_instance].i_offset_first = current_instance.i_offset_first
                                                           + arr_offsets[i] - arr_counts[i];
          arr_instances[i_current_instance].i_offset_last = current_instance.i_offset_first
                                                          + arr_offsets[i] - 1;
          arr_instances[i_current_instance].i_depth = current_instance.i_depth + 1;
          arr_instances[i_current_instance].b_copy = 1;
          ++i_current_instance;
        }
        else if(arr_counts[i] > 0){//we still need to copy data in original array
          memcpy(
            &(arr_elements[current_instance.i_offset_first + arr_offsets[i] - arr_counts[i]]),
            &(arr_elements_copy[current_instance.i_offset_first + arr_offsets[i] - arr_counts[i]]),
            arr_counts[i] * sizeof(t_TSODLULS_sort_element)
          );
        }
      }//end for(int i = 0; i < 256; ++i)
    }

    if(i_current_instance == 0){
      break;
    }

    current_instance = arr_instances[--i_current_instance];
  }//end while(true)

  TSODLULS_free(arr_elements_copy);
  TSODLULS_free(arr_instances);
  return 0;
}//end function TSODLULS_sort_radix8_count()



/**
 * Sorting functions for long nextified strings
 * A stable sorting algorithm for nextified strings based on radix sort with octets digits
 * and counting sort as a subroutine.
 * When the number of elements to sort is at most: 5 for initial sort or 2 for radix sequel, we use insertion sort
 */
int TSODLULS_sort_radix8_count_insertion(
  t_TSODLULS_sort_element* arr_elements,
  size_t i_number_of_elements
){

  int i_number_of_distinct_bytes = 0;
  size_t arr_counts[256];
  size_t arr_offsets[256];
  uint8_t i_current_octet = 0;
  t_TSODLULS_sort_element* arr_elements_copy = NULL;
  t_TSODLULS_radix_instance current_instance;
  t_TSODLULS_radix_instance* arr_instances = NULL;
  size_t i_max_number_of_instances = 0;
  size_t i_current_instance = 0;
  void* p_for_realloc = NULL;
  size_t i_size_for_realloc = 0;
  t_TSODLULS_sort_element tmp_cell;
  t_TSODLULS_sort_element* p_cell1;
  t_TSODLULS_sort_element* p_cell2;

#define min(x, y) ((x) < (y) ? (x) : (y))

  if(i_number_of_elements < 2){
    return 0;//nothing to sort
  }

  if(i_number_of_elements <= 5){
    goto label_insertion_sort;
  }

  current_instance.i_offset_first = 0;
  current_instance.i_offset_last = i_number_of_elements - 1;
  current_instance.i_depth = 0;
  current_instance.b_copy = 0;

  arr_elements_copy = calloc(i_number_of_elements, sizeof(t_TSODLULS_sort_element));
  if(arr_elements_copy == NULL){
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_instances = calloc(8, sizeof(t_TSODLULS_radix_instance));
  if(arr_instances == NULL){
    TSODLULS_free(arr_elements_copy);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  i_max_number_of_instances =  8;

  while(1){
    //we initialize the counters
    for(int i = 0; i < 256; ++i){
      arr_counts[i] = 0;
      arr_offsets[i] = 0;
    }
    //we count the bytes with a certain value
    if(current_instance.b_copy){
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements_copy[i].s_key[current_instance.i_depth];
        ++arr_counts[i_current_octet];
      }
    }
    else{
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements[i].s_key[current_instance.i_depth];
        ++arr_counts[i_current_octet];
      }
    }
    //we deduce the offsets
    i_number_of_distinct_bytes = 0;
    for(int i = 0; i < 255; ++i){
      arr_offsets[i+1] = arr_offsets[i] + arr_counts[i];
      if(arr_counts[i] > 0){
        ++i_number_of_distinct_bytes;
      }
    }
    if(arr_counts[255] > 0){
      ++i_number_of_distinct_bytes;
    }

    if(i_number_of_distinct_bytes == 1){
      ++current_instance.i_depth;
      if(current_instance.b_copy){
        //if we are done sorting this instance and all deeper subinstances
        if(arr_elements_copy[current_instance.i_offset_first].i_key_size <= current_instance.i_depth){
          memcpy(
              &(arr_elements[current_instance.i_offset_first]),
              &(arr_elements_copy[current_instance.i_offset_first]),
              (current_instance.i_offset_last - current_instance.i_offset_first + 1) * sizeof(t_TSODLULS_sort_element)
          );
          if(i_current_instance == 0){
            break;
          }
          current_instance = arr_instances[--i_current_instance];
        }
      }
      else{
        //if we are done sorting this instance and all deeper subinstances
        if(arr_elements[current_instance.i_offset_first].i_key_size <= current_instance.i_depth){
          if(i_current_instance == 0){
            break;
          }
          current_instance = arr_instances[--i_current_instance];
        }
      }
      continue;
    }

    if(current_instance.b_copy){
      //sorting
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements_copy[i].s_key[current_instance.i_depth];
        arr_elements[current_instance.i_offset_first + arr_offsets[i_current_octet]] = arr_elements_copy[i];
        ++arr_offsets[i_current_octet];
      }
      //instances creation
      for(int i = 0; i < 256; ++i){
        if(arr_counts[i] > 2
          //for nextified strings this test can be done on only one element
          && arr_elements[current_instance.i_offset_first + arr_offsets[i] - 1].i_key_size
               > current_instance.i_depth + 1
        ){
          //we allocate more space for the new instance if necessary
          if(i_current_instance == i_max_number_of_instances - 1){
            i_size_for_realloc = i_max_number_of_instances * 2 * sizeof(t_TSODLULS_radix_instance);
            //if we cannot allocate more
            if(i_size_for_realloc <= i_max_number_of_instances * sizeof(t_TSODLULS_radix_instance)){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            p_for_realloc = realloc(arr_instances, i_size_for_realloc);
            if(p_for_realloc == NULL){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            arr_instances = (t_TSODLULS_radix_instance*)p_for_realloc;
            i_max_number_of_instances *= 2;
          }
          arr_instances[i_current_instance].i_offset_first = current_instance.i_offset_first
                                                           + arr_offsets[i] - arr_counts[i];
          arr_instances[i_current_instance].i_offset_last = current_instance.i_offset_first
                                                          + arr_offsets[i] - 1;
          arr_instances[i_current_instance].i_depth = current_instance.i_depth + 1;
          arr_instances[i_current_instance].b_copy = 0;
          ++i_current_instance;
        }
        else if(arr_counts[i] == 2
          //for nextified strings this test can be done on only one element
          && arr_elements[current_instance.i_offset_first + arr_offsets[i] - 1].i_key_size
               > current_instance.i_depth + 1
        ){
          p_cell1 = &(arr_elements[current_instance.i_offset_first + arr_offsets[i] - 2]);
          p_cell2 = p_cell1 + 1;
          for(size_t j = current_instance.i_depth + 1, j_max = min(p_cell1->i_key_size, p_cell2->i_key_size); j < j_max; ++j){
            if(p_cell2->s_key[j] < p_cell1->s_key[j]){
              tmp_cell = *p_cell1; *p_cell1 = *p_cell2; *p_cell2 = tmp_cell;//swapping
              break;
            }
            if(p_cell2->s_key[j] > p_cell1->s_key[j]){
              break;
            }
          }
        }
      }//end for(int i = 0; i < 256; ++i)
    }
    else{
      //sorting
      for(size_t i = current_instance.i_offset_first; i <= current_instance.i_offset_last; ++i){
        i_current_octet = arr_elements[i].s_key[current_instance.i_depth];
        arr_elements_copy[current_instance.i_offset_first + arr_offsets[i_current_octet]] = arr_elements[i];
        ++arr_offsets[i_current_octet];
      }
      //instances creation
      for(int i = 0; i < 256; ++i){
        if(arr_counts[i] > 2
          //for nextified strings this test can be done on only one element
          && arr_elements_copy[current_instance.i_offset_first + arr_offsets[i] - 1].i_key_size
               > current_instance.i_depth + 1
        ){
          //we allocate more space for the new instance if necessary
          if(i_current_instance == i_max_number_of_instances - 1){
            i_size_for_realloc = i_max_number_of_instances * 2 * sizeof(t_TSODLULS_radix_instance);
            //if we cannot allocate more
            if(i_size_for_realloc <= i_max_number_of_instances * sizeof(t_TSODLULS_radix_instance)){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            p_for_realloc = realloc(arr_instances, i_size_for_realloc);
            if(p_for_realloc == NULL){
              TSODLULS_free(arr_elements_copy);
              TSODLULS_free(arr_instances);
              return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
            }
            arr_instances = (t_TSODLULS_radix_instance*)p_for_realloc;
            i_max_number_of_instances *= 2;
          }
          arr_instances[i_current_instance].i_offset_first = current_instance.i_offset_first
                                                           + arr_offsets[i] - arr_counts[i];
          arr_instances[i_current_instance].i_offset_last = current_instance.i_offset_first
                                                          + arr_offsets[i] - 1;
          arr_instances[i_current_instance].i_depth = current_instance.i_depth + 1;
          arr_instances[i_current_instance].b_copy = 1;
          ++i_current_instance;
        }
        else if(arr_counts[i] > 0){//we still need to copy data in original array
          memcpy(
            &(arr_elements[current_instance.i_offset_first + arr_offsets[i] - arr_counts[i]]),
            &(arr_elements_copy[current_instance.i_offset_first + arr_offsets[i] - arr_counts[i]]),
            arr_counts[i] * sizeof(t_TSODLULS_sort_element)
          );
          if(arr_counts[i] == 2
            //for nextified strings this test can be done on only one element
            && arr_elements_copy[current_instance.i_offset_first + arr_offsets[i] - 1].i_key_size
                 > current_instance.i_depth + 1
          ){
            p_cell1 = &(arr_elements[current_instance.i_offset_first + arr_offsets[i] - 2]);
            p_cell2 = p_cell1 + 1;
            for(size_t j = current_instance.i_depth + 1, j_max = min(p_cell1->i_key_size, p_cell2->i_key_size); j < j_max; ++j){
              if(p_cell2->s_key[j] < p_cell1->s_key[j]){
                tmp_cell = *p_cell1; *p_cell1 = *p_cell2; *p_cell2 = tmp_cell;//swapping
                break;
              }
              if(p_cell2->s_key[j] > p_cell1->s_key[j]){
                break;
              }
            }
          }
        }
      }//end for(int i = 0; i < 256; ++i)
    }

    if(i_current_instance == 0){
      break;
    }

    current_instance = arr_instances[--i_current_instance];
  }//end while(true)

  TSODLULS_free(arr_elements_copy);
  TSODLULS_free(arr_instances);

  return 0;//this is costly to scan again all memory with insertion sort at the end
  //If there is at most 5 elements that's what we use, otherwise we continue what we started with radix
  // dealing with the case of two elements above

  /* Once the arr_elements array is partially sorted by radix sort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. arr_elements points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */

  label_insertion_sort:
  {
    t_TSODLULS_sort_element* const end_ptr = &arr_elements[(i_number_of_elements - 1)];
    t_TSODLULS_sort_element* tmp_ptr = arr_elements;
    t_TSODLULS_sort_element* thresh = min(end_ptr, arr_elements + 5);
    t_TSODLULS_sort_element* run_ptr;
    size_t i;
    size_t i_max;

    /* Find smallest element in first threshold and place it at the
       array's beginning.  This is the smallest array element,
       and the operation speeds up insertion sort's inner loop. */

    for(run_ptr = tmp_ptr + 1; run_ptr <= thresh; ++run_ptr){
      //nextified strings
      for(i = 0, i_max = min(run_ptr->i_key_size, tmp_ptr->i_key_size); i < i_max; ++i){
        if(run_ptr->s_key[i] < tmp_ptr->s_key[i]){
          tmp_ptr = run_ptr;
          break;
        }
        if(run_ptr->s_key[i] > tmp_ptr->s_key[i]){
          break;
        }
      }
    }

    if(tmp_ptr != arr_elements){
      tmp_cell = *tmp_ptr; *tmp_ptr = *arr_elements; *arr_elements = tmp_cell;//swapping
    }

    /* Insertion sort, running from left-hand-side up to right-hand-side.  */
    run_ptr = arr_elements + 1;
    while((++run_ptr) <= end_ptr){
      tmp_ptr = run_ptr - 1;
      int b_do_while = 0;
      //nextified strings
      for(i = 0, i_max = min(run_ptr->i_key_size, tmp_ptr->i_key_size); i < i_max; ++i){
        if(run_ptr->s_key[i] < tmp_ptr->s_key[i]){
          b_do_while = 1;
          break;
        }
        if(run_ptr->s_key[i] > tmp_ptr->s_key[i]){
          break;
        }
      }
      //while(run_ptr->i_key < tmp_ptr->i_key){
      while(b_do_while){
        --tmp_ptr;
        b_do_while = 0;
        //nextified strings
        for(i = 0, i_max = min(run_ptr->i_key_size, tmp_ptr->i_key_size); i < i_max; ++i){
          if(run_ptr->s_key[i] < tmp_ptr->s_key[i]){
            b_do_while = 1;
            break;
          }
          if(run_ptr->s_key[i] > tmp_ptr->s_key[i]){
            break;
          }
        }
      }
      ++tmp_ptr;
      if(tmp_ptr != run_ptr){
        t_TSODLULS_sort_element* trav;

        trav = run_ptr + 1;
        while(--trav >= run_ptr){
          tmp_cell = *trav;
          t_TSODLULS_sort_element* hi;
          t_TSODLULS_sort_element* lo;
          for (hi = lo = trav; (--lo) >= tmp_ptr; hi = lo){
            *hi = *lo;
          }
          *hi = tmp_cell;
        }
      }
    }
  }

  return 0;
}//end function TSODLULS_sort_radix8_count_insertion()



