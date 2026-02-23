#include "pch.h"
#include "merge_sort.h"
#include <vector>

namespace
{
    void merge_ranges(std::vector<int>& a, int left, int mid, int right)
    {
        const int n1 = mid - left + 1;
        const int n2 = right - mid;

        std::vector<int> L(n1);
        std::vector<int> R(n2);

        for (int i = 0; i < n1; ++i) L[i] = a[left + i];
        for (int j = 0; j < n2; ++j) R[j] = a[mid + 1 + j];

        int i = 0, j = 0, k = left;

        while (i < n1 && j < n2)
        {
            if (L[i] <= R[j]) a[k++] = L[i++];
            else              a[k++] = R[j++];
        }

        while (i < n1) a[k++] = L[i++];
        while (j < n2) a[k++] = R[j++];
    }

    void merge_sort_impl(std::vector<int>& a, int left, int right)
    {
        if (left >= right) return;

        const int mid = left + (right - left) / 2;
        merge_sort_impl(a, left, mid);
        merge_sort_impl(a, mid + 1, right);
        merge_ranges(a, left, mid, right);
    }
}

namespace sorting
{
    void merge_sort(std::vector<int>& arr)
    {
        if (!arr.empty())
            merge_sort_impl(arr, 0, static_cast<int>(arr.size()) - 1);
    }
}