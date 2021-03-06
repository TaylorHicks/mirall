/*
 * Copyright (C) by Krzesimir Nowak <krzesimir@endocode.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include <QVariant>

#include "wizard/owncloudshibbolethcredspage.h"
#include "mirall/theme.h"
#include "wizard/owncloudwizardcommon.h"
#include "creds/shibbolethcredentials.h"
#include "creds/shibboleth/shibbolethwebview.h"

namespace Mirall
{

OwncloudShibbolethCredsPage::OwncloudShibbolethCredsPage()
    : AbstractCredentialsWizardPage(),
      _browser(0),
      _cookie(),
      _afterInitialSetup(false)
{}

void OwncloudShibbolethCredsPage::setupBrowser()
{
    if (_browser) {
        return;
    }
    _browser = new ShibbolethWebView(QUrl(field("OCUrl").toString().simplified()));
    connect(_browser, SIGNAL(shibbolethCookieReceived(QNetworkCookie)),
            this, SLOT(slotShibbolethCookieReceived(QNetworkCookie)));
    connect(_browser, SIGNAL(viewHidden()),
            this, SLOT(slotViewHidden()));

    _browser->show();
    _browser->setFocus();
}

void OwncloudShibbolethCredsPage::setVisible(bool visible)
{
    if (!_afterInitialSetup) {
        QWizardPage::setVisible(visible);
        return;
    }

    if (isVisible() == visible) {
        return;
    }
    if (_browser) {
        disposeBrowser();
    }
    if (visible) {
        setupBrowser();
        wizard()->hide();
    } else {
        wizard()->show();
    }
}

void OwncloudShibbolethCredsPage::initializePage()
{
    _afterInitialSetup = true;
    _cookie = QNetworkCookie();
}

void OwncloudShibbolethCredsPage::disposeBrowser()
{
    if (_browser) {
        disconnect(_browser, SIGNAL(viewHidden()),
                   this, SLOT(slotViewHidden()));
        disconnect(_browser, SIGNAL(shibbolethCookieReceived(QNetworkCookie)),
                   this, SLOT(slotShibbolethCookieReceived(QNetworkCookie)));
        _browser->hide();
        _browser->deleteLater();
        _browser = 0;
    }
}

int OwncloudShibbolethCredsPage::nextId() const
{
  return WizardCommon::Page_AdvancedSetup;
}

void OwncloudShibbolethCredsPage::setConnected()
{
    wizard()->show();
}

AbstractCredentials* OwncloudShibbolethCredsPage::getCredentials() const
{
    return new ShibbolethCredentials(_cookie);
}

void OwncloudShibbolethCredsPage::slotShibbolethCookieReceived(const QNetworkCookie& cookie)
{
    disposeBrowser();
    _cookie = cookie;
    emit connectToOCUrl(field("OCUrl").toString().simplified());
}

void OwncloudShibbolethCredsPage::slotViewHidden()
{
    disposeBrowser();
    wizard()->back();
    wizard()->show();
}

} // ns Mirall
