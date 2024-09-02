import asyncio
import re
import requests
from urllib.parse import quote
from telegram import Update
from telegram.ext import Application, CommandHandler, MessageHandler, filters, ContextTypes

# Regular expression for IPv4
ipv4_pattern = re.compile(r"\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b")

# Bot token
TELEGRAM_BOT_TOKEN = "7402264307:AAFEspHU4_GEye3vcofwajdRrAVsf_Wf6SM"

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await update.message.reply_text("Kirimkan IP address (IPv4) dalam pesan untuk diproses.")

async def ipinfo(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    message_text = update.message.text.strip()
    
    # Find all IPv4 addresses in the message
    ips = ipv4_pattern.findall(message_text)
    
    if not ips:
        await update.message.reply_text("Format IP tidak didukung, mohon kirim IP address (IPv4) yang valid.")
        return
    
    for ip in ips:
        try:
            # Send "Checking IP Information..."
            message = await update.message.reply_text(f"⌛️")
            
            # Encode the URL
            encoded_url = quote(f"https://xsmnet.buzz/api?key=xsm&ip={ip}")
            response = requests.get(f"https://api.allorigins.win/raw?url={encoded_url}")
            response.raise_for_status()
            
            data = response.json()
            
            # Collect necessary fields, ensuring they exist or setting to 'N/A' if missing
            ip_address = data.get('ip', 'N/A')
            origin_ip = data.get('originIp', 'N/A')
            isp = data.get('isp', 'N/A')
            org = data.get('org', 'N/A')
            country = data.get('country', 'N/A')
            city = data.get('city', 'N/A')
            proxy_status = data.get('proxyStatus', 'N/A')
            
            # Format the result with quotes
            result = f"```\n"
            result += f"IP           : {ip_address}\n"
            result += f"OriginIp     : {origin_ip}\n"
            result += f"ISP          : {isp}\n"
            result += f"Org          : {org}\n"
            result += f"Country      : {country}\n"
            result += f"City         : {city}\n"
            result += f"Proxy Status : {proxy_status}\n"
            result += f"```\n"
            
            # Edit message to show IP information
            await message.edit_text(result, parse_mode='Markdown')  # Edit message to show IP information
            
            # Delay for 2 seconds before processing the next IP
            await asyncio.sleep(2)
            
        except requests.exceptions.RequestException as e:
            error_message = f"Kesalahan mendapatkan info untuk IP {ip}"
            await update.message.reply_text(error_message)
        except ValueError as e:
            error_message = f"Kesalahan parsing JSON untuk IP {ip}"
            await update.message.reply_text(error_message)

def main() -> None:
    print("Bot Running Now 🛰️")
    
    application = Application.builder().token(TELEGRAM_BOT_TOKEN).build()
    application.add_handler(CommandHandler("start", start))
    application.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, ipinfo))
    application.run_polling()

if __name__ == "__main__":
    main()
