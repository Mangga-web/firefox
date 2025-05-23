/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

//! Specified values.
//!
//! TODO(emilio): Enhance docs.

use super::computed::transform::DirectionVector;
use super::computed::{Context, ToComputedValue};
use super::generics::grid::ImplicitGridTracks as GenericImplicitGridTracks;
use super::generics::grid::{GridLine as GenericGridLine, TrackBreadth as GenericTrackBreadth};
use super::generics::grid::{TrackList as GenericTrackList, TrackSize as GenericTrackSize};
use super::generics::transform::IsParallelTo;
use super::generics::{self, GreaterThanOrEqualToOne, NonNegative};
use super::{CSSFloat, CSSInteger};
use crate::context::QuirksMode;
use crate::parser::{Parse, ParserContext};
use crate::values::specified::calc::CalcNode;
use crate::values::{serialize_atom_identifier, serialize_number, AtomString};
use crate::{Atom, Namespace, One, Prefix, Zero};
use cssparser::{Parser, Token};
use std::fmt::{self, Write};
use std::ops::Add;
use style_traits::values::specified::AllowedNumericType;
use style_traits::{CssWriter, ParseError, SpecifiedValueInfo, StyleParseErrorKind, ToCss};

pub use self::align::{AlignContent, AlignItems, AlignSelf, ContentDistribution};
pub use self::align::{JustifyContent, JustifyItems, JustifySelf, SelfAlignment};
pub use self::angle::{AllowUnitlessZeroAngle, Angle};
pub use self::animation::{
    AnimationComposition, AnimationDirection, AnimationDuration, AnimationFillMode,
    AnimationIterationCount, AnimationName, AnimationPlayState, AnimationTimeline, ScrollAxis,
    TimelineName, TransitionBehavior, TransitionProperty, ViewTimelineInset, ViewTransitionName,
};
pub use self::background::{BackgroundRepeat, BackgroundSize};
pub use self::basic_shape::FillRule;
pub use self::border::{
    BorderCornerRadius, BorderImageRepeat, BorderImageSideWidth, BorderImageSlice,
    BorderImageWidth, BorderRadius, BorderSideWidth, BorderSpacing, BorderStyle, LineWidth,
};
pub use self::box_::{
    Appearance, BaselineSource, BreakBetween, BreakWithin, Clear, Contain, ContainIntrinsicSize,
    ContainerName, ContainerType, ContentVisibility, Display, Float, LineClamp, Overflow,
    OverflowAnchor, OverflowClipBox, OverscrollBehavior, Perspective, PositionProperty, Resize,
    ScrollSnapAlign, ScrollSnapAxis, ScrollSnapStop, ScrollSnapStrictness, ScrollSnapType,
    ScrollbarGutter, TouchAction, VerticalAlign, WillChange, WillChangeBits, WritingModeProperty,
    Zoom,
};
pub use self::color::{
    Color, ColorOrAuto, ColorPropertyValue, ColorScheme, ForcedColorAdjust, PrintColorAdjust,
};
pub use self::column::ColumnCount;
pub use self::counters::{Content, ContentItem, CounterIncrement, CounterReset, CounterSet};
pub use self::easing::TimingFunction;
pub use self::effects::{BoxShadow, Filter, SimpleShadow};
pub use self::flex::FlexBasis;
pub use self::font::{FontFamily, FontLanguageOverride, FontPalette, FontStyle};
pub use self::font::{FontFeatureSettings, FontVariantLigatures, FontVariantNumeric};
pub use self::font::{
    FontSize, FontSizeAdjust, FontSizeAdjustFactor, FontSizeKeyword, FontStretch, FontSynthesis, FontSynthesisStyle,
};
pub use self::font::{FontVariantAlternates, FontWeight};
pub use self::font::{FontVariantEastAsian, FontVariationSettings, LineHeight};
pub use self::font::{MathDepth, MozScriptMinSize, MozScriptSizeMultiplier, XLang, XTextScale};
pub use self::image::{EndingShape as GradientEndingShape, Gradient, Image, ImageRendering};
pub use self::length::{AbsoluteLength, AnchorSizeFunction, CalcLengthPercentage, CharacterWidth};
pub use self::length::{FontRelativeLength, Length, LengthOrNumber, NonNegativeLengthOrNumber};
pub use self::length::{LengthOrAuto, LengthPercentage, LengthPercentageOrAuto};
pub use self::length::{Margin, MaxSize, Size};
pub use self::length::{NoCalcLength, ViewportPercentageLength, ViewportVariant};
pub use self::length::{
    NonNegativeLength, NonNegativeLengthPercentage, NonNegativeLengthPercentageOrAuto,
};
#[cfg(feature = "gecko")]
pub use self::list::ListStyleType;
pub use self::list::Quotes;
pub use self::motion::{OffsetPath, OffsetPosition, OffsetRotate};
pub use self::outline::OutlineStyle;
pub use self::page::{PageName, PageOrientation, PageSize, PageSizeOrientation, PaperSize};
pub use self::percentage::{NonNegativePercentage, Percentage};
pub use self::position::AnchorFunction;
pub use self::position::AnchorName;
pub use self::position::AnchorScope;
pub use self::position::AspectRatio;
pub use self::position::Inset;
pub use self::position::PositionAnchor;
pub use self::position::PositionTryFallbacks;
pub use self::position::PositionTryOrder;
pub use self::position::PositionVisibility;
pub use self::position::{GridAutoFlow, GridTemplateAreas, Position, PositionOrAuto};
pub use self::position::{MasonryAutoFlow, MasonryItemOrder, MasonryPlacement};
pub use self::position::{PositionArea, PositionAreaKeyword};
pub use self::position::{PositionComponent, ZIndex};
pub use self::ratio::Ratio;
pub use self::rect::NonNegativeLengthOrNumberRect;
pub use self::resolution::Resolution;
pub use self::svg::{DProperty, MozContextProperties};
pub use self::svg::{SVGLength, SVGOpacity, SVGPaint};
pub use self::svg::{SVGPaintOrder, SVGStrokeDashArray, SVGWidth, VectorEffect};
pub use self::svg_path::SVGPathData;
pub use self::text::{HyphenateCharacter, HyphenateLimitChars};
pub use self::text::RubyPosition;
pub use self::text::TextAlignLast;
pub use self::text::TextUnderlinePosition;
pub use self::text::{InitialLetter, LetterSpacing, LineBreak, TextAlign, TextIndent};
pub use self::text::{OverflowWrap, TextEmphasisPosition, TextEmphasisStyle, WordBreak};
pub use self::text::{TextAlignKeyword, TextDecorationLine, TextOverflow, WordSpacing};
pub use self::text::{TextDecorationLength, TextDecorationSkipInk, TextJustify, TextTransform};
pub use self::time::Time;
pub use self::transform::{Rotate, Scale, Transform};
pub use self::transform::{TransformBox, TransformOrigin, TransformStyle, Translate};
#[cfg(feature = "gecko")]
pub use self::ui::CursorImage;
pub use self::ui::{
    BoolInteger, Cursor, Inert, MozTheme, PointerEvents, ScrollbarColor, UserFocus, UserInput,
    UserSelect,
};
pub use super::generics::grid::GridTemplateComponent as GenericGridTemplateComponent;

pub mod align;
pub mod angle;
pub mod animation;
pub mod background;
pub mod basic_shape;
pub mod border;
#[path = "box.rs"]
pub mod box_;
pub mod calc;
pub mod color;
pub mod column;
pub mod counters;
pub mod easing;
pub mod effects;
pub mod flex;
pub mod font;
pub mod grid;
pub mod image;
pub mod intersection_observer;
pub mod length;
pub mod list;
pub mod motion;
pub mod outline;
pub mod page;
pub mod percentage;
pub mod position;
pub mod ratio;
pub mod rect;
pub mod resolution;
pub mod source_size_list;
pub mod svg;
pub mod svg_path;
pub mod table;
pub mod text;
pub mod time;
pub mod transform;
pub mod ui;
pub mod url;

/// <angle> | <percentage>
/// https://drafts.csswg.org/css-values/#typedef-angle-percentage
#[allow(missing_docs)]
#[derive(Clone, Copy, Debug, MallocSizeOf, PartialEq, SpecifiedValueInfo, ToCss, ToShmem)]
pub enum AngleOrPercentage {
    Percentage(Percentage),
    Angle(Angle),
}

impl AngleOrPercentage {
    fn parse_internal<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
        allow_unitless_zero: AllowUnitlessZeroAngle,
    ) -> Result<Self, ParseError<'i>> {
        if let Ok(per) = input.try_parse(|i| Percentage::parse(context, i)) {
            return Ok(AngleOrPercentage::Percentage(per));
        }

        Angle::parse_internal(context, input, allow_unitless_zero).map(AngleOrPercentage::Angle)
    }

    /// Allow unitless angles, used for conic-gradients as specified by the spec.
    /// https://drafts.csswg.org/css-images-4/#valdef-conic-gradient-angle
    pub fn parse_with_unitless<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        AngleOrPercentage::parse_internal(context, input, AllowUnitlessZeroAngle::Yes)
    }
}

impl Parse for AngleOrPercentage {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        AngleOrPercentage::parse_internal(context, input, AllowUnitlessZeroAngle::No)
    }
}

/// Parse a `<number>` value, with a given clamping mode.
fn parse_number_with_clamping_mode<'i, 't>(
    context: &ParserContext,
    input: &mut Parser<'i, 't>,
    clamping_mode: AllowedNumericType,
) -> Result<Number, ParseError<'i>> {
    let location = input.current_source_location();
    match *input.next()? {
        Token::Number { value, .. } if clamping_mode.is_ok(context.parsing_mode, value) => {
            Ok(Number {
                value,
                calc_clamping_mode: None,
            })
        },
        Token::Function(ref name) => {
            let function = CalcNode::math_function(context, name, location)?;
            let value = CalcNode::parse_number(context, input, function)?;
            Ok(Number {
                value,
                calc_clamping_mode: Some(clamping_mode),
            })
        },
        ref t => Err(location.new_unexpected_token_error(t.clone())),
    }
}

/// A CSS `<number>` specified value.
///
/// https://drafts.csswg.org/css-values-3/#number-value
#[derive(Clone, Copy, Debug, MallocSizeOf, PartialOrd, ToShmem)]
pub struct Number {
    /// The numeric value itself.
    value: CSSFloat,
    /// If this number came from a calc() expression, this tells how clamping
    /// should be done on the value.
    calc_clamping_mode: Option<AllowedNumericType>,
}

impl Parse for Number {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        parse_number_with_clamping_mode(context, input, AllowedNumericType::All)
    }
}

impl PartialEq<Number> for Number {
    fn eq(&self, other: &Number) -> bool {
        if self.calc_clamping_mode != other.calc_clamping_mode {
            return false;
        }

        self.value == other.value || (self.value.is_nan() && other.value.is_nan())
    }
}

impl Number {
    /// Returns a new number with the value `val`.
    #[inline]
    fn new_with_clamping_mode(
        value: CSSFloat,
        calc_clamping_mode: Option<AllowedNumericType>,
    ) -> Self {
        Self {
            value,
            calc_clamping_mode,
        }
    }

    /// Returns this percentage as a number.
    pub fn to_percentage(&self) -> Percentage {
        Percentage::new_with_clamping_mode(self.value, self.calc_clamping_mode)
    }

    /// Returns a new number with the value `val`.
    #[inline]
    pub fn new(val: CSSFloat) -> Self {
        Self::new_with_clamping_mode(val, None)
    }

    /// Returns whether this number came from a `calc()` expression.
    #[inline]
    pub fn was_calc(&self) -> bool {
        self.calc_clamping_mode.is_some()
    }

    /// Returns the numeric value, clamped if needed.
    #[inline]
    pub fn get(&self) -> f32 {
        crate::values::normalize(
            self.calc_clamping_mode
                .map_or(self.value, |mode| mode.clamp(self.value)),
        )
        .min(f32::MAX)
        .max(f32::MIN)
    }

    #[allow(missing_docs)]
    pub fn parse_non_negative<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Number, ParseError<'i>> {
        parse_number_with_clamping_mode(context, input, AllowedNumericType::NonNegative)
    }

    #[allow(missing_docs)]
    pub fn parse_at_least_one<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Number, ParseError<'i>> {
        parse_number_with_clamping_mode(context, input, AllowedNumericType::AtLeastOne)
    }

    /// Clamp to 1.0 if the value is over 1.0.
    #[inline]
    pub fn clamp_to_one(self) -> Self {
        Number {
            value: self.value.min(1.),
            calc_clamping_mode: self.calc_clamping_mode,
        }
    }
}

impl ToComputedValue for Number {
    type ComputedValue = CSSFloat;

    #[inline]
    fn to_computed_value(&self, _: &Context) -> CSSFloat {
        self.get()
    }

    #[inline]
    fn from_computed_value(computed: &CSSFloat) -> Self {
        Number {
            value: *computed,
            calc_clamping_mode: None,
        }
    }
}

impl ToCss for Number {
    fn to_css<W>(&self, dest: &mut CssWriter<W>) -> fmt::Result
    where
        W: Write,
    {
        serialize_number(self.value, self.calc_clamping_mode.is_some(), dest)
    }
}

impl IsParallelTo for (Number, Number, Number) {
    fn is_parallel_to(&self, vector: &DirectionVector) -> bool {
        use euclid::approxeq::ApproxEq;
        // If a and b is parallel, the angle between them is 0deg, so
        // a x b = |a|*|b|*sin(0)*n = 0 * n, |a x b| == 0.
        let self_vector = DirectionVector::new(self.0.get(), self.1.get(), self.2.get());
        self_vector
            .cross(*vector)
            .square_length()
            .approx_eq(&0.0f32)
    }
}

impl SpecifiedValueInfo for Number {}

impl Add for Number {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        Self::new(self.get() + other.get())
    }
}

impl Zero for Number {
    #[inline]
    fn zero() -> Self {
        Self::new(0.)
    }

    #[inline]
    fn is_zero(&self) -> bool {
        self.get() == 0.
    }
}

impl From<Number> for f32 {
    #[inline]
    fn from(n: Number) -> Self {
        n.get()
    }
}

impl From<Number> for f64 {
    #[inline]
    fn from(n: Number) -> Self {
        n.get() as f64
    }
}

/// A Number which is >= 0.0.
pub type NonNegativeNumber = NonNegative<Number>;

impl Parse for NonNegativeNumber {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        parse_number_with_clamping_mode(context, input, AllowedNumericType::NonNegative)
            .map(NonNegative::<Number>)
    }
}

impl One for NonNegativeNumber {
    #[inline]
    fn one() -> Self {
        NonNegativeNumber::new(1.0)
    }

    #[inline]
    fn is_one(&self) -> bool {
        self.get() == 1.0
    }
}

impl NonNegativeNumber {
    /// Returns a new non-negative number with the value `val`.
    pub fn new(val: CSSFloat) -> Self {
        NonNegative::<Number>(Number::new(val.max(0.)))
    }

    /// Returns the numeric value.
    #[inline]
    pub fn get(&self) -> f32 {
        self.0.get()
    }
}

/// An Integer which is >= 0.
pub type NonNegativeInteger = NonNegative<Integer>;

impl Parse for NonNegativeInteger {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        Ok(NonNegative(Integer::parse_non_negative(context, input)?))
    }
}

/// A Number which is >= 1.0.
pub type GreaterThanOrEqualToOneNumber = GreaterThanOrEqualToOne<Number>;

impl Parse for GreaterThanOrEqualToOneNumber {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        parse_number_with_clamping_mode(context, input, AllowedNumericType::AtLeastOne)
            .map(GreaterThanOrEqualToOne::<Number>)
    }
}

/// <number> | <percentage>
///
/// Accepts only non-negative numbers.
#[allow(missing_docs)]
#[derive(Clone, Copy, Debug, MallocSizeOf, PartialEq, SpecifiedValueInfo, ToCss, ToShmem)]
pub enum NumberOrPercentage {
    Percentage(Percentage),
    Number(Number),
}

impl NumberOrPercentage {
    fn parse_with_clamping_mode<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
        type_: AllowedNumericType,
    ) -> Result<Self, ParseError<'i>> {
        if let Ok(per) =
            input.try_parse(|i| Percentage::parse_with_clamping_mode(context, i, type_))
        {
            return Ok(NumberOrPercentage::Percentage(per));
        }

        parse_number_with_clamping_mode(context, input, type_).map(NumberOrPercentage::Number)
    }

    /// Parse a non-negative number or percentage.
    pub fn parse_non_negative<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        Self::parse_with_clamping_mode(context, input, AllowedNumericType::NonNegative)
    }

    /// Convert the number or the percentage to a number.
    pub fn to_percentage(self) -> Percentage {
        match self {
            Self::Percentage(p) => p,
            Self::Number(n) => n.to_percentage(),
        }
    }

    /// Convert the number or the percentage to a number.
    pub fn to_number(self) -> Number {
        match self {
            Self::Percentage(p) => p.to_number(),
            Self::Number(n) => n,
        }
    }
}

impl Parse for NumberOrPercentage {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        Self::parse_with_clamping_mode(context, input, AllowedNumericType::All)
    }
}

/// A non-negative <number> | <percentage>.
pub type NonNegativeNumberOrPercentage = NonNegative<NumberOrPercentage>;

impl NonNegativeNumberOrPercentage {
    /// Returns the `100%` value.
    #[inline]
    pub fn hundred_percent() -> Self {
        NonNegative(NumberOrPercentage::Percentage(Percentage::hundred()))
    }

    /// Return a particular number.
    #[inline]
    pub fn new_number(n: f32) -> Self {
        NonNegative(NumberOrPercentage::Number(Number::new(n)))
    }
}

impl Parse for NonNegativeNumberOrPercentage {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        Ok(NonNegative(NumberOrPercentage::parse_non_negative(
            context, input,
        )?))
    }
}

/// The value of Opacity is <alpha-value>, which is "<number> | <percentage>".
/// However, we serialize the specified value as number, so it's ok to store
/// the Opacity as Number.
#[derive(
    Clone, Copy, Debug, MallocSizeOf, PartialEq, PartialOrd, SpecifiedValueInfo, ToCss, ToShmem,
)]
pub struct Opacity(Number);

impl Parse for Opacity {
    /// Opacity accepts <number> | <percentage>, so we parse it as NumberOrPercentage,
    /// and then convert into an Number if it's a Percentage.
    /// https://drafts.csswg.org/cssom/#serializing-css-values
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        let number = NumberOrPercentage::parse(context, input)?.to_number();
        Ok(Opacity(number))
    }
}

impl ToComputedValue for Opacity {
    type ComputedValue = CSSFloat;

    #[inline]
    fn to_computed_value(&self, context: &Context) -> CSSFloat {
        let value = self.0.to_computed_value(context);
        if context.for_smil_animation {
            // SMIL expects to be able to interpolate between out-of-range
            // opacity values.
            value
        } else {
            value.min(1.0).max(0.0)
        }
    }

    #[inline]
    fn from_computed_value(computed: &CSSFloat) -> Self {
        Opacity(Number::from_computed_value(computed))
    }
}

/// A specified `<integer>`, either a simple integer value or a calc expression.
/// Note that a calc expression may not actually be an integer; it will be rounded
/// at computed-value time.
///
/// <https://drafts.csswg.org/css-values/#integers>
#[derive(Clone, Copy, Debug, MallocSizeOf, PartialEq, PartialOrd, ToShmem)]
pub enum Integer {
    /// A literal integer value.
    Literal(CSSInteger),
    /// A calc expression, whose value will be rounded later if necessary.
    Calc(CSSFloat),
}

impl Zero for Integer {
    #[inline]
    fn zero() -> Self {
        Self::new(0)
    }

    #[inline]
    fn is_zero(&self) -> bool {
        *self == 0
    }
}

impl One for Integer {
    #[inline]
    fn one() -> Self {
        Self::new(1)
    }

    #[inline]
    fn is_one(&self) -> bool {
        *self == 1
    }
}

impl PartialEq<i32> for Integer {
    fn eq(&self, value: &i32) -> bool {
        self.value() == *value
    }
}

impl Integer {
    /// Trivially constructs a new `Integer` value.
    pub fn new(val: CSSInteger) -> Self {
        Self::Literal(val)
    }

    /// Returns the (rounded) integer value associated with this value.
    pub fn value(&self) -> CSSInteger {
        match *self {
            Self::Literal(i) => i,
            Self::Calc(n) => (n + 0.5).floor() as CSSInteger,
        }
    }

    /// Trivially constructs a new integer value from a `calc()` expression.
    fn from_calc(val: CSSFloat) -> Self {
        Self::Calc(val)
    }
}

impl Parse for Integer {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        let location = input.current_source_location();
        match *input.next()? {
            Token::Number {
                int_value: Some(v), ..
            } => Ok(Integer::new(v)),
            Token::Function(ref name) => {
                let function = CalcNode::math_function(context, name, location)?;
                let result = CalcNode::parse_number(context, input, function)?;
                Ok(Integer::from_calc(result))
            },
            ref t => Err(location.new_unexpected_token_error(t.clone())),
        }
    }
}

impl Integer {
    /// Parse an integer value which is at least `min`.
    pub fn parse_with_minimum<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
        min: i32,
    ) -> Result<Integer, ParseError<'i>> {
        let value = Integer::parse(context, input)?;
        // FIXME(emilio): The spec asks us to avoid rejecting it at parse
        // time except until computed value time.
        //
        // It's not totally clear it's worth it though, and no other browser
        // does this.
        if value.value() < min {
            return Err(input.new_custom_error(StyleParseErrorKind::UnspecifiedError));
        }
        Ok(value)
    }

    /// Parse a non-negative integer.
    pub fn parse_non_negative<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Integer, ParseError<'i>> {
        Integer::parse_with_minimum(context, input, 0)
    }

    /// Parse a positive integer (>= 1).
    pub fn parse_positive<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Integer, ParseError<'i>> {
        Integer::parse_with_minimum(context, input, 1)
    }
}

impl ToComputedValue for Integer {
    type ComputedValue = i32;

    #[inline]
    fn to_computed_value(&self, _: &Context) -> i32 {
        self.value()
    }

    #[inline]
    fn from_computed_value(computed: &i32) -> Self {
        Integer::new(*computed)
    }
}

impl ToCss for Integer {
    fn to_css<W>(&self, dest: &mut CssWriter<W>) -> fmt::Result
    where
        W: Write,
    {
        match *self {
            Integer::Literal(i) => i.to_css(dest),
            Integer::Calc(n) => {
                dest.write_str("calc(")?;
                n.to_css(dest)?;
                dest.write_char(')')
            }
        }
    }
}

impl SpecifiedValueInfo for Integer {}

/// A wrapper of Integer, with value >= 1.
pub type PositiveInteger = GreaterThanOrEqualToOne<Integer>;

impl Parse for PositiveInteger {
    #[inline]
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        Integer::parse_positive(context, input).map(GreaterThanOrEqualToOne)
    }
}

/// The specified value of a grid `<track-breadth>`
pub type TrackBreadth = GenericTrackBreadth<LengthPercentage>;

/// The specified value of a grid `<track-size>`
pub type TrackSize = GenericTrackSize<LengthPercentage>;

/// The specified value of a grid `<track-size>+`
pub type ImplicitGridTracks = GenericImplicitGridTracks<TrackSize>;

/// The specified value of a grid `<track-list>`
/// (could also be `<auto-track-list>` or `<explicit-track-list>`)
pub type TrackList = GenericTrackList<LengthPercentage, Integer>;

/// The specified value of a `<grid-line>`.
pub type GridLine = GenericGridLine<Integer>;

/// `<grid-template-rows> | <grid-template-columns>`
pub type GridTemplateComponent = GenericGridTemplateComponent<LengthPercentage, Integer>;

/// rect(...)
pub type ClipRect = generics::GenericClipRect<LengthOrAuto>;

impl Parse for ClipRect {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Self, ParseError<'i>> {
        Self::parse_quirky(context, input, AllowQuirks::No)
    }
}

impl ClipRect {
    /// Parses a rect(<top>, <left>, <bottom>, <right>), allowing quirks.
    fn parse_quirky<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
        allow_quirks: AllowQuirks,
    ) -> Result<Self, ParseError<'i>> {
        input.expect_function_matching("rect")?;

        fn parse_argument<'i, 't>(
            context: &ParserContext,
            input: &mut Parser<'i, 't>,
            allow_quirks: AllowQuirks,
        ) -> Result<LengthOrAuto, ParseError<'i>> {
            LengthOrAuto::parse_quirky(context, input, allow_quirks)
        }

        input.parse_nested_block(|input| {
            let top = parse_argument(context, input, allow_quirks)?;
            let right;
            let bottom;
            let left;

            if input.try_parse(|input| input.expect_comma()).is_ok() {
                right = parse_argument(context, input, allow_quirks)?;
                input.expect_comma()?;
                bottom = parse_argument(context, input, allow_quirks)?;
                input.expect_comma()?;
                left = parse_argument(context, input, allow_quirks)?;
            } else {
                right = parse_argument(context, input, allow_quirks)?;
                bottom = parse_argument(context, input, allow_quirks)?;
                left = parse_argument(context, input, allow_quirks)?;
            }

            Ok(ClipRect {
                top,
                right,
                bottom,
                left,
            })
        })
    }
}

/// rect(...) | auto
pub type ClipRectOrAuto = generics::GenericClipRectOrAuto<ClipRect>;

impl ClipRectOrAuto {
    /// Parses a ClipRect or Auto, allowing quirks.
    pub fn parse_quirky<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
        allow_quirks: AllowQuirks,
    ) -> Result<Self, ParseError<'i>> {
        if let Ok(v) = input.try_parse(|i| ClipRect::parse_quirky(context, i, allow_quirks)) {
            return Ok(generics::GenericClipRectOrAuto::Rect(v));
        }
        input.expect_ident_matching("auto")?;
        Ok(generics::GenericClipRectOrAuto::Auto)
    }
}

/// Whether quirks are allowed in this context.
#[derive(Clone, Copy, PartialEq)]
pub enum AllowQuirks {
    /// Quirks are not allowed.
    No,
    /// Quirks are allowed, in quirks mode.
    Yes,
    /// Quirks are always allowed, used for SVG lengths.
    Always,
}

impl AllowQuirks {
    /// Returns `true` if quirks are allowed in this context.
    pub fn allowed(self, quirks_mode: QuirksMode) -> bool {
        match self {
            AllowQuirks::Always => true,
            AllowQuirks::No => false,
            AllowQuirks::Yes => quirks_mode == QuirksMode::Quirks,
        }
    }
}

/// An attr(...) rule
///
/// `[namespace? `|`]? ident`
#[derive(
    Clone,
    Debug,
    Eq,
    MallocSizeOf,
    PartialEq,
    SpecifiedValueInfo,
    ToComputedValue,
    ToResolvedValue,
    ToShmem,
)]
#[css(function)]
#[repr(C)]
pub struct Attr {
    /// Optional namespace prefix.
    pub namespace_prefix: Prefix,
    /// Optional namespace URL.
    pub namespace_url: Namespace,
    /// Attribute name
    pub attribute: Atom,
    /// Fallback value
    pub fallback: AtomString,
}

impl Parse for Attr {
    fn parse<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Attr, ParseError<'i>> {
        input.expect_function_matching("attr")?;
        input.parse_nested_block(|i| Attr::parse_function(context, i))
    }
}

/// Get the Namespace for a given prefix from the namespace map.
fn get_namespace_for_prefix(prefix: &Prefix, context: &ParserContext) -> Option<Namespace> {
    context.namespaces.prefixes.get(prefix).cloned()
}

/// Try to parse a namespace and return it if parsed, or none if there was not one present
fn parse_namespace<'i, 't>(
    context: &ParserContext,
    input: &mut Parser<'i, 't>,
) -> Result<(Prefix, Namespace), ParseError<'i>> {
    let ns_prefix = match input.next()? {
        Token::Ident(ref prefix) => Some(Prefix::from(prefix.as_ref())),
        Token::Delim('|') => None,
        _ => return Err(input.new_custom_error(StyleParseErrorKind::UnspecifiedError)),
    };

    if ns_prefix.is_some() && !matches!(*input.next_including_whitespace()?, Token::Delim('|')) {
        return Err(input.new_custom_error(StyleParseErrorKind::UnspecifiedError));
    }

    if let Some(prefix) = ns_prefix {
        let ns = match get_namespace_for_prefix(&prefix, context) {
            Some(ns) => ns,
            None => return Err(input.new_custom_error(StyleParseErrorKind::UnspecifiedError)),
        };
        Ok((prefix, ns))
    } else {
        Ok((Prefix::default(), Namespace::default()))
    }
}

impl Attr {
    /// Parse contents of attr() assuming we have already parsed `attr` and are
    /// within a parse_nested_block()
    pub fn parse_function<'i, 't>(
        context: &ParserContext,
        input: &mut Parser<'i, 't>,
    ) -> Result<Attr, ParseError<'i>> {
        // Syntax is `[namespace? '|']? ident [',' fallback]?`
        let namespace = input
            .try_parse(|input| parse_namespace(context, input))
            .ok();
        let namespace_is_some = namespace.is_some();
        let (namespace_prefix, namespace_url) = namespace.unwrap_or_default();

        // If there is a namespace, ensure no whitespace following '|'
        let attribute = Atom::from(if namespace_is_some {
            let location = input.current_source_location();
            match *input.next_including_whitespace()? {
                Token::Ident(ref ident) => ident.as_ref(),
                ref t => return Err(location.new_unexpected_token_error(t.clone())),
            }
        } else {
            input.expect_ident()?.as_ref()
        });

        // Fallback will always be a string value for now as we do not support
        // attr() types yet.
        let fallback = input
            .try_parse(|input| -> Result<AtomString, ParseError<'i>> {
                input.expect_comma()?;
                Ok(input.expect_string()?.as_ref().into())
            })
            .unwrap_or_default();

        Ok(Attr {
            namespace_prefix,
            namespace_url,
            attribute,
            fallback,
        })
    }
}

impl ToCss for Attr {
    fn to_css<W>(&self, dest: &mut CssWriter<W>) -> fmt::Result
    where
        W: Write,
    {
        dest.write_str("attr(")?;
        if !self.namespace_prefix.is_empty() {
            serialize_atom_identifier(&self.namespace_prefix, dest)?;
            dest.write_char('|')?;
        }
        serialize_atom_identifier(&self.attribute, dest)?;

        if !self.fallback.is_empty() {
            dest.write_str(", ")?;
            self.fallback.to_css(dest)?;
        }

        dest.write_char(')')
    }
}
