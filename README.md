# yrp - A Recursive Descent Parser Powered by C++ Template ###

This project is inspired by [yard](https://code.google.com/p/yardparser/).

## 一個簡單的 integer number parsing rule

先來 parse 一個 integer, integer 的 regex 是 `\d+`，用 yrp 來寫是

```c++
struct integer :
    yrp::plus<
        yrp::digit_char
    >
{};
```

`plus` 跟 `digit_char` 都是 yrp 事先已經定義好的 parsing rule，
`plus` 就是 regex 的 `+`，可以用來 match 一個到無限多個被 `<...>` 框住的 parsing rule，
在裡面的 `digit_char` 則是 match 一個數字字元 `0` ~ `9`。

## 一個簡單的 floating number parsing rule

再來 parse 一個 floating number, float number 的 regex 是 `\d*.\d+`，用 yrp 來寫是

```c++
struct digit :
    yrp::seq<
        yrp::star<yrp::digit_char>,
        yrp::char_<L'.'>,
        yrp::plus<yrp::digit_char>
    >
{};
```

## 開始 parse

```c++
int main() {
    // 宣告準備要被 parse 的字串
    std::wstring i = L"123";
    std::wstring f = L"3.14159";

    // 宣告 parser
    yrp::parser<std::wstring::const_iterator> p(i.begin(), i.end());

    // 進行 parse
    bool result = p.parse<integer>();
    std::cout << result << std::endl; // 印出 true
}
```

不過這個簡單的 parser 沒有支援任何 semantic action。semantic action 後面會教，但很爛。

## 內建的 general parsing rule 列表

因為 `yrp` 是用 template 寫成，所以其實 parsing rule 都可以**樣版化**，也就是 parsing rule 只是一種抽象概念，可以透過 template 來多樣化，組合成你要的東西。

一些 parsing 常見的 rule `yrp` 都已經內建了。比方說 `or`, `plus`, `star`, `opt` 等等。

以下是完整的列表：

- `any` 任何字元，類似 regex 的 `.`

- `any_but<Rule>` 除了 `Rule` 以外的任何字元，類似 regex 的 `[^Rule]`
- `at<Rule>` 後面一定要是 `Rule`，會嘗試 match，但不會消耗 `Rule`，類似 `(?=Rule)`
- `not_at<Rule>` 後面不是 `Rule`，不會消耗，類似 `(?!Rule)`
- `eoi` End of input
- `expect<Rule>` 一定要是 `Rule`，否則會丟出一個 `std::logic_error`
- `expect_except<Rule, E>` 一定要是 `Rule`，否則會丟出一個 `E`
- `back_ref<Rule, N>` 把 match 的 `Rule` 設定為第 `N` 的 back reference，類似 regex 的 `()` 與 `\1, \2, ...` 的對應（總共有 9 個 slot 可以用，可以自己改 code 增加）
- `ref_to<N>` reference 到第 `N` 個 group
- `list<Rule, Deli>` 被 `Deli` 分隔的 `Rule` 序列，比方說「被逗號分隔的數字」就是 `list<digit, char_<L','>>`
- `longest<Rule0, Rule1, ...>` 會嘗試 match 每一個 rule，最後選定消耗最多字元的那個，非常罕用
- `not_<Rule>` 好像根本不會用到 = =" 我也不知道當初寫這幹嘛
- `opt<Rule>` 「有或沒有都可以」，regex 的 `?`
- `or<Rule0, Rule1, ...>` regex 的 `|`
- `repeat<Rule, N, M = N>` regex 的 `Rule{N, M}`
- `seq<Rule0, Rule1, ...>` 先 match `Rule0`, 然後 `Rule1`，然後...一定要全部都 match 才是 match。跟 `or` 兩個是最常用的 generic rule
- `star<Rule>` regex 的 `Rule*`
- `plus<Rule>` regex 的 `Rule+`
- `until_pass<Rule>` 什麼都吃，一直到吃掉 `Rule` 為止，就停下來
- `until_at<Rule>` 什麼都吃，一直到吃掉 `Rule` 之前，就停下來

## 內建的 char parsing rule 列表

`yrp` parse 的對象不一定是 `char`, 事實上可以是自訂的 token，當然也可以是 `wchar_t`，`yrp` 因為最常用來 parse 的對象就是 `wchar_t` 所以 `yrp` 也內建一些讓你用來 match `wchar_t` 的 rule：

- `char_<C0, C1, ...>` match 字元 `C0` 或 `C1` 或 ... 類似 regex 的 `[abc...]` 這樣

- `char_<C0, C1, ...>` match 字元 `C0` 或 `C1` 或 ... 字外的任何字元，類似 `[^abc...]` 這樣
- `range_char<B0, E0, B1, E1, ...>` match `B0` 到 `E0` 或 `B1` 到 `E1` 或 ... 類似 `[a-zA-Z...]` 這樣
- `str<C0, C1, ...>` match 由 `C0`, `C1`, ... 組成的字串，講真的這個很難用，卻常常要用，唉
- `new_line_char`   = 換行
- `sign_char`       = `char_<L'+', L'-'> {};`
- `digit_char`      = `range_char<L'0', L'9'> {};`
- `bin_char`        = `char_<L'0', L'1'> {};`
- `oct_char`        = `range_char<L'0', L'7'> {};`
- `hex_char`        = `range_char<L'0', L'9', L'a', L'f', L'A', L'F'> {};`
- `lower_char`      = `range_char<L'a', L'z'> {};`
- `upper_char`      = `range_char<L'A', L'Z'> {};`
- `alpha_char`      = `range_char<L'a', L'z', L'A', L'Z'> {};`
- `alpha_num_char`  = `range_char<L'a', L'z', L'A', L'Z', '0', '9'> {};`
- `head_char`       = `or_<alpha_char, char_<L'_'>> {};`
- `word_char`       = `or_<alpha_num_char, char_<L'_'>> {};`
- `id_word`         = `seq<head_char, star<word_char>> {};`
- `whitespace_char` = `char_<WHITESPACE, HORIZONTAL_TAB> {};`
- `whitespace_seq`  = `plus<whitespace_char> {};`
- `quoted_str<Quote>` 由 `Quote` 框起來的，可以被 `\` escape 的 string，比方說 `quoted_str<L'"'>` 就類似 c-string

## 自訂 parsing rule

當然你也可以自己根據自己的需求來加新的可泛用的 parsing rule ，有兩種新增
parsing rule 的方法，一種比較簡單，也是開發 parser 的的時候，你大部分的時間都會花在這裡

### 組合現有的 parsing rule

比方假如你需要「被一對括弧框起來」的 parsing rule `parenthesis` 可以寫成

```c++
template <typename Rule>
struct parenthesised :
    seq<
        char_<L'('>,
        Rule,
        char_<L')'>
    >
{};
```

注意到上面這個 parsing rule 是個 template，所以他可以被泛化使用，很方便，當你設計自己的 parsing rule 的時候，也請盡量考慮設計成 template，增加可複用性。

或是你想要 parse 一個十六進位的數字 `0[xX][0-9a-fA-F]+`

```c++
struct hex_number :
    yrp::seq<
        yrp::char_<L'0'>,
        yrp::char_<L'x', L'X'>,
        yrp::star<yrp::hex_char>
    >
{};
```

如果寫的原始一點，那就是

```c++
struct hex_number :
    yrp::seq<
        yrp::char_<L'0'>,
        yrp::char_<L'x', L'X'>,
        yrp::star<
            yrp::or<
                ypr::digit_char,
                ypr::range_char<L'a', L'f'>,
                ypr::range_char<L'A', L'F'>
            >
        >
    >
{};
```

### 打造自己特殊需求的 parsing rule

有的時候真的很衰，`yrp` 內建的 parsing rule 沒有一個符合你的需求，
這個時候你就要自己手動 code parsing rule，聽起來很難，但實際上非常簡單，
讓我們看一下 `yrp` 內建的 `star`：

```c++
template <typename Rule>
struct star {
    template <typename Parser>
    static bool match(Parser& p) {
        while(Rule::template match(p));
        return true;
    }
};
```

再來看一下簡化版的 `seq`（實際上是 variadic template recursive 在做）：

```c++
template <typename Rule0, typename Rule1, ...>
struct seq {
    template <typename Parser>
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos(); // 記住 iterator 本來的位置！
        if(!Rule0::template match(p)) {
            p.pos(orig_pos);    // 看這個！如果失敗了，就回到本來的的位置！
            return false;
        }
        if(!Rule1::template match(p)) {
            p.pos(orig_pos);    // 看這個！如果失敗了，就回到本來的的位置！
            return false;
        }
        ...
        return true;
    }
};
```

或是更簡單的 `any_char`

```c++
struct any_char {
    template <typename Parser>
    static bool match(Parser &p) {
        if(p.at_end()) return false;
        p.next();       // 看到這邊！
        return true;
    }
} ;
```

所以規則很簡單：

1. parse 成功：傳回 true

2. parse 失敗：傳回 false

3. 失敗的話，根據你的需要（幾乎都需要，我是想不到不設回去的場合啦）把 parser 的 iterator 設定回原來的地方

4. 成功的話，根據你的需要，呼叫 `p.next()` 把 iterator 往前一步。
   原則就是每個 rule 自己管理好 iterator，所以如果你是呼叫某個 rule 來 parse，
   那你大概就不用管手動 `next()`，因為那個 rule 裡面應該會自己負責做好這件事情，
   就像你寫一般的 recursive descent parser 一樣就對了。
   
   基本上只有處理到 char 層次的 rule 才會需要手動 `next()` 啦，
   因為比較高階的 generic rule 都只是包裝低階的動作，那低階的動作都已經自己顧好 iterator 了。

形式就是照抄，基本上就像是你在寫 recursive decsent parser 一樣，你成功了，就往前走，失敗了，就回到某處。

## Semantic action

在 `yrp` 裡面，semactic action 也是透過 parsing rule 來達成。
比方說我們想要把 parse 到的 int 都存起來，
那我們可以寫一個特別的 parsing rule，有點像最上面範例的 integer：

```c++
struct integer :
    yrp::plus<
        yrp::digit_char
    >
{};
```

現在改成

```c++
struct save_integer {
    static bool match(Parser& p) {
        typename Parser::iterator orig_pos = p.pos(); // 記住 iterator 本來的位置
        if(integer::template match(p)) {
            // 成功了，把字串轉成文字，存到 parser object 裡面
            int result = std::stoi(orig_pos, p.pos());
            p.my_push_back_integer(result);  // my_push_back_integer 這函數哪來的???
            return true;
        } else {
            p.pos(orig_pos);
            return false;
        }
    }
};
```

可以看到，非常的麻煩，所以我們不會真的這樣作。

我們真的作法，比這更麻煩。

`yrp` 目前啦，因為我還沒想到比較漂亮的作法，因為我不想導入 global variable，
所以我把所有的 context 都放在 parser class 裡面。
但不同的 parser 需要處理不同的 semantic action，也有不同的 context，怎麼辦，
所以 user 要自己繼承 parser class，提供 semantic action 所需要的界面。
我們發現「套用 semantic action」其實也是一種 pattern，所以就可以**樣板化**。
實際上 `yrp` 提供了兩個觸動 semantic action 的 rule，一個是直接呼叫 action 的 `just_act`
另外一個是滿足條件才會觸動的 `post_act`。

為了要加上 semantic action 的支援，一定自己繼承一個 parser：

```c++
using WsIterType = std::wstring::const_iterator;
struct IntParser : public yrp::parser<std::wstring::const_iterator>
{
  public:

    // constructor
    IntParser(WsIterType b, WsIterType e)
        : yrp::parser<WsIterType>(b, e)
    {
    }

    // using super's member，該死的 C++，不 using 看不見
    using IterType = typename WsIterType;
    using RangeType = std::pair<IterType, IterType>;
    using typename yrp::parser<IterType>::iterator;
    using typename yrp::parser<IterType>::reference;
    using typename yrp::parser<IterType>::value_type;
    using yrp::parser<IterType>::pos_;
    using yrp::parser<IterType>::elem;
    using yrp::parser<IterType>::pos;
    using yrp::parser<IterType>::begin;
    using yrp::parser<IterType>::end;
    using yrp::parser<IterType>::at_begin;
    using yrp::parser<IterType>::at_end;
    using yrp::parser<IterType>::next;

    // parse
    template <typename Rule>
    bool parse() {
        return Rule::match(*this);
    }
    
    // ↑ 通常到上面為止大多是照抄
    // ↓ 下面開始才是為了 semantic action 寫的
    
    // semantic action 函數們!!!!
    // 一概都拿一對 iterator 當作輸入，不管用的到用不到
    // return bool
    bool my_push_back_integer(RangeType r) {
        int i = std::stoi(r.first, r.second);
        _v.push_back(i);
        return true;
    }
    bool my_clear_vec(RangeType) {
        _v.clear();
        return true;
    }
};
```

更賭懶的來了，因為 parsing rule 只能由 class 組成，所以就沒辦法用 boost 那種 attributed grammar 的作法，
但我也不想，因為一旦導入，就得得入跟 `boost::any`, `boost::variant` 等價的等等 class，而且還要作 `boost::fusion` 等動作，
那就會大幅增加編譯時間。在這邊為了呼叫動作，只好乖乖多走一層，把動作包裝在 class 內部，然後轉派呼叫 parser 提供的 semantic action 函數。

```c++
namespace act {
    struct my_push_back_integer {
        template <typename Parser>
        bool operator()(Parser& p, typename Parser::iterator begin, typename Parser::iterator end) {
            return p.my_push_back_integer({begin, end});
        }
    } ;
}
```

這時候透過 `just_act` 或 `post_act` 來觸動 semantic action。

```c++
// [3,4,5]
struct integer_list :
    yrp::seq<
        yrp::char_<L'['>,
        yrp::just_act<act::my_clear_vec>, // 遇到 '[' 表示新的 list，你可能會想要清掉本來的
        yrp::list<
            yrp::post_act<integer, act::my_push_back_integer>,
            yrp::char_<L','>
        >,
        yrp::char_<L']'>
    >
{};
```

終於大功告成，謝天謝地！我都快寫不下去了！
如果有任何人看完這段還願意使用這個 library，那我覺得你一定跟自己很想不開。

因為那些轉派動作的 act class 實在太 verbose 了，所以我很難得的使用了 macro 來解決。

```c++
#define RANGE_ACT(action_name) \
struct action_name \
{ \
    template <typename Parser> \
    bool operator()(Parser& p, typename Parser::iterator begin, typename Parser::iterator end) { \
        return p.action_name({begin, end}); \
    } \
} ;

RANGE_ACT(my_push_back_integer)
RANGE_ACT(my_clear_vec)
RANGE_ACT(my_print_vec)
```

在 `test.cc` 有一份完整會動的範例碼，不多，請參閱
