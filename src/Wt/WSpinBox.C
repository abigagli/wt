/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WSpinBox>
#include <Wt/WIntValidator>
#include <Wt/WLocale>

#include "DomElement.h"

namespace Wt {

WSpinBox::WSpinBox(WContainerWidget *parent)
  : WAbstractSpinBox(parent),
    value_(-1),
    min_(0),
    max_(99),
    step_(1)
{ 
  setValidator(createValidator());
  setValue(0);
}

void WSpinBox::setValue(int value)
{
  if (value_ != value) {
    value_ = value;
    setText(textFromValue());
  }
}

void WSpinBox::setMinimum(int minimum)
{
  min_ = minimum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WSpinBox::setMaximum(int maximum)
{
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WSpinBox::setRange(int minimum, int maximum)
{
  min_ = minimum;
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WSpinBox::setSingleStep(int step)
{
  step_ = step;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

int WSpinBox::decimals() const
{
  return 0;
}

std::string WSpinBox::jsMinMaxStep() const 
{
  return boost::lexical_cast<std::string>(min_) + ","
    + boost::lexical_cast<std::string>(max_) + ","
    + boost::lexical_cast<std::string>(step_);
}

void WSpinBox::updateDom(DomElement& element, bool all)
{
  if (all || changed_) {
    if (nativeControl()) {
      element.setAttribute("min", boost::lexical_cast<std::string>(min_));
      element.setAttribute("max", boost::lexical_cast<std::string>(max_));
      element.setAttribute("step", boost::lexical_cast<std::string>(step_));
    } else {
      /* Make sure the JavaScript validator is loaded */
      WIntValidator v;
      v.javaScriptValidate();
    }
  }

  WAbstractSpinBox::updateDom(element, all);
}

void WSpinBox::signalConnectionsChanged()
{
  if (valueChanged_.isConnected() && !valueChangedConnection_) {
    valueChangedConnection_ = true;
    changed().connect(this, &WSpinBox::onChange);
  }

  WAbstractSpinBox::signalConnectionsChanged();
}

void WSpinBox::onChange()
{
  valueChanged_.emit(value());
}

WValidator *WSpinBox::createValidator()
{
  WIntValidator *validator = new WIntValidator();
  validator->setRange(min_, max_);
  return validator;
}

WString WSpinBox::textFromValue() const
{
  if (nativeControl())    
    return WLocale::currentLocale().toString(value_);
  else {
    std::string text = prefix().toUTF8()
      + WLocale::currentLocale().toString(value_).toUTF8()
      + suffix().toUTF8();

    return WString::fromUTF8(text);
  }
}

bool WSpinBox::parseNumberValue(const std::string& text)
{
  try {
    value_ = WLocale::currentLocale().toInt(WT_USTRING::fromUTF8(text));
    return true;
  } catch (boost::bad_lexical_cast &e) {
    return false;
  }
}

WValidator::Result WSpinBox::validateRange() const
{
  WIntValidator validator;
  validator.setRange(min_, max_);
  return validator.validate(WString("{1}").arg(value_));
}

}
