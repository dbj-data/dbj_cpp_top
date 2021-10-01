#ifndef DBJ_NOT_A_VECTOR_INCLUDED
#define DBJ_NOT_A_VECTOR_INCLUDED

// (c) 2021 by dbj@dbj.org
// https://godbolt.org/z/dh8GP7qr3
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdexcept>
#include <string>
#include <type_traits>

// dynamic buffer of trivially copyable types
// by no means finished either in design or in implementation
template <typename T>
class not_a_vector {
  // https://en.cppreference.com/w/cpp/string/byte/memcpy
  static_assert(std::is_trivially_copyable_v<T>,
                "\n\nnot_a_vector<>T -- realloc can handle only trivially "
                "copiable types!\n\n");

  constexpr static size_t initial_capacity_ = 0xFF;
  constexpr static size_t capacity_increment_ = 2;
  size_t size_ = 0;
  size_t capacity_ = 0;
  T *arr_ = nullptr;

 public:
  // contructors and destructors
  not_a_vector() noexcept : size_(0), capacity_(0), arr_(nullptr) {
    resize(initial_capacity_);
  }
  ~not_a_vector() noexcept {
    if (arr_ != nullptr) {
      resize(0);
      size_ = 0;
      capacity_ = 0;
    }
  }

  //
  not_a_vector(not_a_vector const &other_) noexcept
      : size_(other_.size_), 
      capacity_(other_.capacity_), 
      arr_(nullptr) 
{
    resize(capacity_);
    memcpy(arr_, other_.arr_, capacity_);
}

  not_a_vector &operator=(not_a_vector const &other_) noexcept {
    resize(0);
    size_ = other_.size_;
    capacity_ = other_.capacity_;
    resize(capacity_);
    memcpy(arr_, other_.arr_, capacity_);
  }

  not_a_vector(not_a_vector &&other_) 
      : size_(other_.size_), capacity_(other_.capacity_), arr_(other_.arr_) {
    other_.arr_ = nullptr;
  }

  not_a_vector &operator=(not_a_vector &&other_)  {
    resize(0);
    size_ = other_.size_;
    capacity_ = other_.capacity_;
    arr_ = other_.arr_;
    other_.arr_ = nullptr;  // do not forget
    return *this;
  }

  ///////////////////////////////////////////////////////

  T &operator[](size_t idx) {
    if (idx > this->size_) {
      errno = EINVAL;
      perror("Bad index!");
      exit(EXIT_FAILURE);
    }

    return this->arr_[idx];
  }

  const T &operator[](size_t idx) const noexcept {
    if (idx > this->size_) {
      errno = EINVAL;
      perror("Bad index!");
      exit(EXIT_FAILURE);
    }

    return this->arr_[idx];
  }

  const T *const data(void) const noexcept { return this->arr_; }
  T *const data(void) noexcept { return this->arr_; }

  // change value functions
  void push_back(/*const*/ T value) {
    if (size_ >= capacity_) resize(capacity_ * capacity_increment_);
    arr_[size_] = value;
    size_++;
  }
  // better resize
  void resize(std::size_t newSize) {
    if (newSize == 0) {       // this check is not strictly needed,
      std::free(this->arr_);  // but zero-size realloc is deprecated in C
      this->arr_ = nullptr;
    } else {
      if (void *mem = std::realloc(this->arr_, newSize))
        this->arr_ = static_cast<T *>(mem);
      else {
        errno = ENOMEM;
        perror("Not enough memory");
        exit(EXIT_FAILURE);
      }
    }
    this->capacity_ = newSize;
  }
};  //////////////////////////////////////////////////////////

#ifdef DBJ_ON_GODBOLT

#define SX(F_, X_) printf("\n(%6d) %s : " F_, __LINE__, (#X_), (X_))

// https://stackoverflow.com/a/45249531/10870835
extern "C"
inline void random_string(int l, char *s) {
    for (int c; c=rand()%62,
     (*(s++) = (c+"07="[(c+16)/26])*(l-->0)););
}

static auto hammer = [](auto instance_) {
  decltype(instance_) tempy_ = instance_;
  return tempy_;
};

int main() {
  not_a_vector<char> vec_;

  char string_[128] = {0};
  random_string(128,string_);
  memcpy(vec_.data(), string_, 128);
  SX("\"%s\"", hammer(vec_).data());
  return 42;
}

#endif // DBJ_ON_GODBOLT

#endif // DBJ_NOT_A_VECTOR_INCLUDED
