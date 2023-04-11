///////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2023.                        //
//  Distributed under the Boost Software License,                //
//  Version 1.0. (See accompanying file LICENSE_1_0.txt          //
//  or copy at http://www.boost.org/LICENSE_1_0.txt)             //
///////////////////////////////////////////////////////////////////

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <examples/example_uintwide_t.h>
#include <math/wide_integer/uintwide_t.h>

namespace example014_pi_spigot
{
  template<const std::uint32_t ResultDigit,
           const std::uint32_t LoopDigit,
           typename UnsignedSmallType,
           typename UnsignedLargeType>
  class pi_spigot
  {
  private:
    using unsigned_small_type = UnsignedSmallType;
    using unsigned_large_type = UnsignedLargeType;

    static constexpr auto result_digit() noexcept -> std::uint32_t { return ResultDigit; }
    static constexpr auto loop_digit  () noexcept -> std::uint32_t { return LoopDigit; }

    static_assert(result_digit() <= static_cast<std::uint32_t>(UINT16_C(10011)),
                  "Error: result_digit exceeds its limit of 10,011");

    static_assert(std::numeric_limits<unsigned_small_type>::digits * 2 == std::numeric_limits<unsigned_large_type>::digits,
                  "Error: unsigned_large_type must be exactly twice as wide as unsigned_small_type");

    static_assert((!std::numeric_limits<unsigned_small_type>::is_signed),
                  "Error: unsigned_small_type must be unsigned");

    static_assert((!std::numeric_limits<unsigned_large_type>::is_signed),
                  "Error: unsigned_large_type must be unsigned");

    static constexpr auto input_scale(std::uint32_t x) noexcept -> std::uint32_t
    {
      return
        static_cast<std::uint32_t>
        (
            static_cast<std::uint32_t>
            (
                x
              * static_cast<std::uint32_t>
                (
                    static_cast<std::uint32_t>(static_cast<std::uint32_t>(UINT8_C(10) * loop_digit()) / static_cast<std::uint32_t>(UINT8_C(3)))
                  + static_cast<std::uint32_t>(UINT8_C(1))
                )
            )
          / loop_digit()
        );
    }

  public:
    static constexpr auto get_output_static_size() noexcept -> std::uint32_t { return result_digit(); }

    static constexpr auto get_input_static_size() noexcept -> std::uint32_t { return input_scale(get_output_static_size()); }

    using input_container_type = std::vector<std::uint32_t>;

    using output_value_type = std::uint8_t;

    WIDE_INTEGER_CONSTEXPR pi_spigot() = default; // LCOV_EXCL_LINE

    WIDE_INTEGER_CONSTEXPR pi_spigot(const pi_spigot&) = delete;

    WIDE_INTEGER_CONSTEXPR pi_spigot(pi_spigot&&) = delete;

    ~pi_spigot() = default; // LCOV_EXCL_LINE

    WIDE_INTEGER_CONSTEXPR auto operator=(const pi_spigot&) -> pi_spigot& = delete;

    WIDE_INTEGER_CONSTEXPR auto operator=(pi_spigot&&) -> pi_spigot& = delete;

    WIDE_INTEGER_NODISCARD WIDE_INTEGER_CONSTEXPR auto get_operation_count() const noexcept -> std::uintmax_t { return my_operation_count; }

    template<typename OutputIteratorType>
    auto calculate(OutputIteratorType output_first) -> void
    {
      // Use pi_spigot::calculate() to calculate result_digit
      // decimal digits of pi.

      // The caller is responsible for providing the output memory
      // for the result of pi.

      // The input memory used for internal calculation details
      // is managed by the pi_spigot class itself.

      if(my_pi_in.empty())
      {
        constexpr auto input_size = static_cast<typename input_container_type::size_type>(get_input_static_size());

        my_pi_in.resize(input_size, static_cast<typename input_container_type::value_type>(UINT8_C(0)));
      }

      auto val_c = static_cast<unsigned_small_type>(static_cast<unsigned>(UINT8_C(0)));

      my_output_count    = static_cast<std::uint32_t>(UINT8_C(0));
      my_operation_count = static_cast<std::uintmax_t>(UINT8_C(0));

      WIDE_INTEGER_CONSTEXPR auto local_pow10 = static_cast<unsigned_large_type>(pow10(loop_digit()));

      // Operation count Mathematica(R), example for loop_digit=9.
      // Sum[Floor[((d - j) (Floor[((10 9)/3)] + 1))/9], {j, 0, Floor[d/9] 9, 9}]

      for(auto j = static_cast<std::uint32_t>(UINT8_C(0));
               j < result_digit(); // NOLINT(altera-id-dependent-backward-branch)
               j = static_cast<std::uint32_t>(j + loop_digit()))
      {
        auto val_d = static_cast<unsigned_large_type>(UINT8_C(0));

        const auto ilim = input_scale(result_digit() - j);

        for(auto   i = static_cast<std::uint32_t>(INT8_C(0));
                   i < ilim; // NOLINT(altera-id-dependent-backward-branch)
                 ++i)
        {
          const auto my_index =
            static_cast<std::uint32_t>
            (
                static_cast<std::uint32_t>(ilim - static_cast<std::uint32_t>(UINT8_C(1)))
              - i
            );

          const auto di =
            ((j == static_cast<std::uint32_t>(UINT8_C(0)))
              ? static_cast<unsigned_large_type>(d_init())
              : static_cast<unsigned_large_type>(my_pi_in[my_index]));

          val_d += (di * local_pow10);

          const auto val_b =
            static_cast<std::uint32_t>
            (
                static_cast<std::uint32_t>
                (
                  my_index * static_cast<std::uint32_t>(UINT8_C(2))
                )
              + static_cast<std::uint32_t>(UINT8_C(1))
            );

          my_pi_in[my_index] = static_cast<std::uint32_t>(val_d % val_b);

          val_d /= val_b;

          if(my_index > static_cast<std::uint32_t>(UINT8_C(1)))
          {
            val_d *= my_index;
          }

          ++my_operation_count;
        }

        // Parse the next digits of pi, where the group has loop_digit digits.
        // If loop_digit is 4, for instance, then successive groups
        // of digits have a form such as: 3141, 5926, ..., etc.

        const auto next_digits =
          static_cast<unsigned_small_type>
          (
            val_c + static_cast<unsigned_small_type>(val_d / local_pow10)
          );

        val_c = static_cast<unsigned_small_type>(val_d % local_pow10);

        const auto n =
          (std::min)
          (
            loop_digit(),
            static_cast<std::uint32_t>(result_digit() - j)
          );

        auto scale10 = pow10(loop_digit() - UINT32_C(1));

        for(auto i = static_cast<std::size_t>(UINT8_C(0)); i < static_cast<std::size_t>(n); ++i) // NOLINT(altera-id-dependent-backward-branch)
        {
          using local_diff_type = typename std::iterator_traits<OutputIteratorType>::difference_type;

          const auto out_index =
            static_cast<local_diff_type>
            (
               static_cast<std::size_t>(static_cast<std::size_t>(j) + i)
            );

          output_first[out_index] =
            static_cast<output_value_type>
            (
              static_cast<unsigned_small_type>(static_cast<unsigned_small_type>(next_digits / scale10) % UINT32_C(10))
            );

          scale10 = static_cast<unsigned_small_type>(scale10 / UINT32_C(10));
        }

        my_output_count += n;
      }
    }

  private:
    static input_container_type my_pi_in; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    std::uintmax_t my_operation_count { }; // NOLINT(readability-identifier-naming)
    std::uint32_t  my_output_count    { }; // NOLINT(readability-identifier-naming)

    static WIDE_INTEGER_CONSTEXPR auto pow10(std::uint32_t n) -> unsigned_small_type // NOLINT(misc-no-recursion)
    {
      return
      (
        (n == static_cast<std::uint32_t>(UINT8_C(0)))
          ? static_cast<unsigned_small_type>(UINT32_C(1))
          : static_cast<unsigned_small_type>(pow10(n - static_cast<std::uint32_t>(UINT8_C(1))) * UINT32_C(10))
      );
    }

    static WIDE_INTEGER_CONSTEXPR auto d_init() -> unsigned_small_type
    {
      return
        static_cast<unsigned_small_type>
        (
          pow10(loop_digit()) / static_cast<unsigned>(UINT8_C(5))
        );
    }
  };

  template<const std::uint32_t ResultDigit,
           const std::uint32_t LoopDigit,
           typename UnsignedSmallType,
           typename UnsignedLargeType>
  typename pi_spigot<ResultDigit, LoopDigit, UnsignedSmallType, UnsignedLargeType>::input_container_type pi_spigot<ResultDigit, LoopDigit, UnsignedSmallType, UnsignedLargeType>::my_pi_in { }; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables,hicpp-uppercase-literal-suffix,readability-uppercase-literal-suffix)

  const std::array<const char*, static_cast<std::size_t>(UINT8_C(12))> pi_control_data =
  {
    "3",
    "1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989",
    "3809525720106548586327886593615338182796823030195203530185296899577362259941389124972177528347913151557485724245415069595082953311686172785588907509838175463746493931925506040092770167113900984882401285836160356370766010471018194295559619894676783744944825537977472684710404753464620804668425906949129331367702898915210475216205696602405803815019351125338243003558764024749647326391419927260426992279678235478163600934172164121992458631503028618297455570674983850549458858692699569092721079750930295532116534498720275596023648066549911988183479775356636980742654252786255181841757467289097777279380008164706001614524919217321721477235014144197356854816136115735255213347574184946843852332390739414333454776241686251898356948556209921922218427255025425688767179049460165346680498862723279178608578438382796797668145410095388378636095068006422512520511739298489608412848862694560424196528502221066118630674427862203919494504712371378696095636437191728746776465757396241389086583264599581339047802759009",
    "9465764078951269468398352595709825822620522489407726719478268482601476990902640136394437455305068203496252451749399651431429809190659250937221696461515709858387410597885959772975498930161753928468138268683868942774155991855925245953959431049972524680845987273644695848653836736222626099124608051243884390451244136549762780797715691435997700129616089441694868555848406353422072225828488648158456028506016842739452267467678895252138522549954666727823986456596116354886230577456498035593634568174324112515076069479451096596094025228879710893145669136867228748940560101503308617928680920874760917824938589009714909675985261365549781893129784821682998948722658804857564014270477555132379641451523746234364542858444795265867821051141354735739523113427166102135969536231442952484937187110145765403590279934403742007310578539062198387447808478489683321445713868751943506430218453191048481005370614680674919278191197939952061419663428754440643745123718192179998391015919561814675142691239748940907186494231961",
    "5679452080951465502252316038819301420937621378559566389377870830390697920773467221825625996615014215030680384477345492026054146659252014974428507325186660021324340881907104863317346496514539057962685610055081066587969981635747363840525714591028970641401109712062804390397595156771577004203378699360072305587631763594218731251471205329281918261861258673215791984148488291644706095752706957220917567116722910981690915280173506712748583222871835209353965725121083579151369882091444210067510334671103141267111369908658516398315019701651511685171437657618351556508849099898599823873455283316355076479185358932261854896321329330898570642046752590709154814165498594616371802709819943099244889575712828905923233260972997120844335732654893823911932597463667305836041428138830320382490375898524374417029132765618093773444030707469211201913020330380197621101100449293215160842444859637669838952286847831235526582131449576857262433441893039686426243410773226978028073189154411010446823252716201052652272111660396",
    "6655730925471105578537634668206531098965269186205647693125705863566201855810072936065987648611791045334885034611365768675324944166803962657978771855608455296541266540853061434443185867697514566140680070023787765913440171274947042056223053899456131407112700040785473326993908145466464588079727082668306343285878569830523580893306575740679545716377525420211495576158140025012622859413021647155097925923099079654737612551765675135751782966645477917450112996148903046399471329621073404375189573596145890193897131117904297828564750320319869151402870808599048010941214722131794764777262241425485454033215718530614228813758504306332175182979866223717215916077166925474873898665494945011465406284336639379003976926567214638530673609657120918076383271664162748888007869256029022847210403172118608204190004229661711963779213375751149595015660496318629472654736425230817703675159067350235072835405670403867435136222247715891504953098444893330963408780769325993978054193414473774418426312986080998886874132604721",
    "5695162396586457302163159819319516735381297416772947867242292465436680098067692823828068996400482435403701416314965897940924323789690706977942236250822168895738379862300159377647165122893578601588161755782973523344604281512627203734314653197777416031990665541876397929334419521541341899485444734567383162499341913181480927777103863877343177207545654532207770921201905166096280490926360197598828161332316663652861932668633606273567630354477628035045077723554710585954870279081435624014517180624643626794561275318134078330336254232783944975382437205835311477119926063813346776879695970309833913077109870408591337464144282277263465947047458784778720192771528073176790770715721344473060570073349243693113835049316312840425121925651798069411352801314701304781643788518529092854520116583934196562134914341595625865865570552690496520985803385072242648293972858478316305777756068887644624824685792603953527734803048029005876075825104747091643961362676044925627420420832085661190625454337213153595845068772460",
    "2901618766795240616342522577195429162991930645537799140373404328752628889639958794757291746426357455254079091451357111369410911939325191076020825202618798531887705842972591677813149699009019211697173727847684726860849003377024242916513005005168323364350389517029893922334517220138128069650117844087451960121228599371623130171144484640903890644954440061986907548516026327505298349187407866808818338510228334508504860825039302133219715518430635455007668282949304137765527939751754613953984683393638304746119966538581538420568533862186725233402830871123282789212507712629463229563989898935821167456270102183564622013496715188190973038119800497340723961036854066431939509790190699639552453005450580685501956730229219139339185680344903982059551002263535361920419947455385938102343955449597783779023742161727111723643435439478221818528624085140066604433258885698670543154706965747458550332323342107301545940516553790686627333799585115625784322988273723198987571415957811196358330059408730681216028764962867",
    "4460477464915995054973742562690104903778198683593814657412680492564879855614537234786733039046883834363465537949864192705638729317487233208376011230299113679386270894387993620162951541337142489283072201269014754668476535761647737946752004907571555278196536213239264061601363581559074220202031872776052772190055614842555187925303435139844253223415762336106425063904975008656271095359194658975141310348227693062474353632569160781547818115284366795706110861533150445212747392454494542368288606134084148637767009612071512491404302725386076482363414334623518975766452164137679690314950191085759844239198629164219399490723623464684411739403265918404437805133389452574239950829659122850855582157250310712570126683024029295252201187267675622041542051618416348475651699981161410100299607838690929160302884002691041407928862150784245167090870006992821206604183718065355672525325675328612910424877618258297651579598470356222629348600341587229805349896502262917487882027342092222453398562647669149055628425039127",
    "5771028402799806636582548892648802545661017296702664076559042909945681506526530537182941270336931378517860904070866711496558343434769338578171138645587367812301458768712660348913909562009939361031029161615288138437909904231747336394804575931493140529763475748119356709110137751721008031559024853090669203767192203322909433467685142214477379393751703443661991040337511173547191855046449026365512816228824462575916333039107225383742182140883508657391771509682887478265699599574490661758344137522397096834080053559849175417381883999446974867626551658276584835884531427756879002909517028352971634456212964043523117600665101241200659755851276178583829204197484423608007193045761893234922927965019875187212726750798125547095890455635792122103334669749923563025494780249011419521238281530911407907386025152274299581807247162591668545133312394804947079119153267343028244186041426363954800044800267049624820179289647669758318327131425170296923488962766844032326092752496035799646925650493681836090032380929345",
    "9588970695365349406034021665443755890045632882250545255640564482465151875471196218443965825337543885690941130315095261793780029741207665147939425902989695946995565761218656196733786236256125216320862869222103274889218654364802296780705765615144632046927906821207388377814233562823608963208068222468012248261177185896381409183903673672220888321513755600372798394004152970028783076670944474560134556417254370906979396122571429894671543578468788614445812314593571984922528471605049221242470141214780573455105008019086996033027634787081081754501193071412233908663938339529425786905076431006383519834389341596131854347546495569781038293097164651438407007073604112373599843452251610507027056235266012764848308407611830130527932054274628654036036745328651057065874882256981579367897669742205750596834408697350201410206723585020072452256326513410559240190274216248439140359989535394590944070469120914093870012645600162374288021092764579310657922955249887275846101264836999892256959688159205600101655256375678",
    "5667227966198857827948488558343975187445455129656344348039664205579829368043522027709842942325330225763418070394769941597915945300697521482933665556615678736400536665641654732170439035213295435291694145990416087532018683793702348886894791510716378529023452924407736594956305100742108714261349745956151384987137570471017879573104229690666702144986374645952808243694457897723300487647652413390759204340196340391147320233807150952220106825634274716460243354400515212669324934196739770415956837535551667302739007497297363549645332888698440611964961627734495182736955882207573551766515898551909866653935494810688732068599075407923424023009259007017319603622547564789406475483466477604114632339056513433068449539790709030234604614709616968868850140834704054607429586991382966824681857103188790652870366508324319744047718556789348230894310682870272280973624809399627060747264553992539944280811373694338872940630792615959954626246297070625948455690347119729964090894180595343932512362355081349490043642785271",
  };

  inline auto pi_control_string() -> std::string
  {
    auto str = std::string { };

    for(auto pstr : pi_control_data) // NOLINT(llvm-qualified-auto,readability-qualified-auto)
    {
      str.insert(str.length(), pstr); // LCOV_EXCL_LINE
    }

    return str;
  }
} // namespace example014_pi_spigot

#if defined(WIDE_INTEGER_NAMESPACE)
auto WIDE_INTEGER_NAMESPACE::math::wide_integer::example014_pi_spigot_wide() -> bool
#else
auto ::math::wide_integer::example014_pi_spigot_wide() -> bool
#endif
{
  #if defined(WIDE_INTEGER_NAMESPACE)
  using unsigned_small_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uint256_t;
  using unsigned_large_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uint512_t;
  #else
  using unsigned_small_type = ::math::wide_integer::uint256_t;
  using unsigned_large_type = ::math::wide_integer::uint512_t;
  #endif

  using pi_spigot_type = example014_pi_spigot::pi_spigot<static_cast<std::uint32_t>(UINT16_C(10001)),
                                                         static_cast<std::uint32_t>(std::numeric_limits<unsigned_small_type>::digits10),
                                                         unsigned_small_type,
                                                         unsigned_large_type>;

  using output_container_type = std::vector<std::uint8_t>;

  output_container_type pi_out(pi_spigot_type::get_output_static_size());

  pi_spigot_type().calculate(pi_out.begin());

  const auto result_is_ok =
    std::equal(pi_out.cbegin(),
               pi_out.cend(),
               example014_pi_spigot::pi_control_string().cbegin(),
               [](const std::uint8_t& by, const char& c)
               {
                 const auto by_val_to_check =
                   static_cast<std::uint8_t>
                   (
                       static_cast<std::uint8_t>(c)
                     - static_cast<std::uint8_t>(UINT8_C(0x30))
                   );

                 return (by == by_val_to_check);
                 });

  return result_is_ok;
}

// Enable this if you would like to activate this main() as a standalone example.
#if defined(WIDE_INTEGER_STANDALONE_EXAMPLE014_PI_SPIGOT_WIDE)

#include <iomanip>
#include <iostream>

auto main() -> int
{
  #if defined(WIDE_INTEGER_NAMESPACE)
  const auto result_is_ok = WIDE_INTEGER_NAMESPACE::math::wide_integer::example011_pi_spigot_wide();
  #else
  const auto result_is_ok = ::math::wide_integer::example011_pi_spigot_wide();
  #endif

  std::cout << "result_is_ok: " << std::boolalpha << result_is_ok << std::endl;

  return (result_is_ok ? 0 : -1);
}

#endif
